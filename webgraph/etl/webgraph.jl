using Stage, GZip, StrPack

log = Log(STDERR)
first_rtsvd_call = true

# -------------------------------------------------------------------------------------------------------------------------
# Utilities
# -------------------------------------------------------------------------------------------------------------------------
function getfile(url, fn; expected_size = -1)
  if filesize(fn) != expected_size
    @timer "downloading $fn from $url" download(url, fn)
  else
    @info "$fn already downloaded, using local version."
  end
end

zopen(f::Function, fn) = ismatch(r"^.*\.gz$", fn) ? gzopen(f, fn) : open(f, fn)

function zcountlines(fn)
  f      = ismatch(r"^.*\.gz$", fn) ? gzopen(fn) : open(fn)
  nlines = countlines(f)
  close(f)
  return nlines
end

# -------------------------------------------------------------------------------------------------------------------------
# Graph Type
# -------------------------------------------------------------------------------------------------------------------------
type Graph
  names   :: Array{String}
  sources :: Array{Int32} # source and dests are sorted and can be used to construct a CSC or CSR sparse matrix quickly
  dests   :: Array{Int32}
end

type CSR
  names :: Array{String}
  rowptr :: Array{Int32}
  colidx :: Array{Int32}
end

Graph() = Graph((String)[],
                (Int32)[],
                (Int32)[])

function GraphPre(nodefn, arcfn)
  g = Graph()

  # read the index
  @timer "reading of node lengths" len = zcountlines(nodefn)
  g.names = Array(String, len)

  @timer "reading node names" zopen(nodefn) do f
    for (i, line) in enumerate(eachline(f))
      l = split(strip(line), '\t')
      g.names[i] = l[1]
    end
  end
  @info "read $(length(g.names)) arcs."

  # read arcs
  @timer "reading arc lengths" len = zcountlines(arcfn)

  g.sources = Array(Int32, len)
  g.dests   = Array(Int32, len)
  @timer "reading arcs" zopen(arcfn) do f
    for (i, arcl) in enumerate(eachline(f))
      arc          = split(strip(arcl[2]), '\t')
      g.sources[i] = int32(arc[1])
      g.dests[i]   = int32(arc[2])
    end
  end
  @info "read $(length(g.sources)) arcs."

  return g
end

function Graph(nodefn, arcfn)
  g = Graph()

  # read the index
  @timer "reading node names" zopen(nodefn) do f
    for l in map(x -> split(strip(x), '\t'), eachline(f))
      push!(g.names, l[1])
    end
  end
  @info "read $(length(g.names)) nodes."

  # test
  #@timer log "[test] name2tld" tnames = [ name2tld(g.names[i]) for i = 1:1_000_000 ]
  #@info log "tnames = $(length(tnames))"
  #@timer log "[test] splittld" nnames = [ splittld(g.names[i]) for i = 1:1_000_000 ]
  #@info log "tnames = $(length(nnames))"

  # read arcs
  @timer "reading arc lengths" len = zcountlines(arcfn)

  g.sources = Array(Int32, len)
  g.dests   = Array(Int32, len)
  gc_disable()

  @timer "reading arcs" zopen(arcfn) do f
    try
      print(log, "")
      for (i, line) in enumerate(eachline(f))
        arc = split(line, '\t')
        g.sources[i] = int64(arc[1]) + 1
        g.dests[i] = int64(arc[2]) + 1
        if i % 200_000 == 0
          print!(log, ".")
        end
        if i % 10_000_000 == 0
          println!(log, @sprintf(" %30d", i))
          gc_enable()
          gc()
          gc_disable()
          print(log, "")
        end
      end
    finally
      gc_enable()
      println!(log, "")
    end
  end
  @info "done. read $(length(g.sources)) arcs."

  return g
end

global const pattern = r"^.*\.(.*)$"

function name2tld(name :: String)
  m = match(pattern, name)
  return string(m == nothing ? name : m.captures[1])
end

function splittld(name :: String)
  va = split(name, '.')
  return va[end]
end

function tldcluster(graph :: Graph)
  @timer "names -> tld" tnames = [ name2tld(graph.names[i]) for i = 1:length(graph.names) ]

  # analyze node names
  tldindex = (String=>Int32)[]
  tldnames = (String)[]
  tidx     = 1
  @timer "making name index" begin
    for t in tnames
      if !(t in keys(tldindex))
        tldindex[t] = tidx
        tidx       += 1
        push!(tldnames, t)
      end
    end
  end

  # gather arcs
  gc_disable()
  arcset   = [ Set{Int32}() for x = 1:length(tldnames) ]
  arcindex = 1
  @timer "making arcsets" begin
    for (idx, tldname) in enumerate(tnames)
      srcidx = tldindex[tldname]
      while arcindex < length(graph.sources) && graph.sources[arcindex] == idx
        target          = tnames[graph.dests[arcindex]]
        push!(arcset[srcidx], tldindex[target])
        arcindex       += 1
      end
    end
  end
  gc_enable()

  # construct final arcset
  numarcs = reduce(+, map(x -> length(x), arcset))
  srcs    = Array(Int32, numarcs) 
  dests   = Array(Int32, numarcs) 
  ai      = 1
  @timer "final arcs" begin
    for i = 1:length(arcset), j = 1:length(arcset[i])
      srcs[ai]  = i
      dests[ai] = j
      ai       += 1
    end
  end

  return Graph(tldnames, srcs, dests)
end

function CSR(g :: Graph)
  prevsrc  = 0
  rowptr   = Int32[]
  dests    = Int32[]
  for i = 1:length(g.sources)
    if g.sources[i] != prevsrc
      for j = (prevsrc+1):g.sources[i]
        push!(rowptr, i) #NOTE: empty rows have the same idx as next row
      end
      prevsrc = g.sources[i]
    end
  end
  while length(rowptr) < g.sources[end]
    push!(rowptr, g.sources[end])
  end
  return CSR(g.names, rowptr-1, g.dests-1)
end

# -------------------------------------------------------------------------------------------------------------------------
# TD
# -------------------------------------------------------------------------------------------------------------------------
immutable TDEnv
  name         :: Ptr{Uint8}
  eval         :: Ptr{Any}
  invoke0      :: Ptr{Any}
  invoke1      :: Ptr{Any}
  invoke2      :: Ptr{Any}
  invoke3      :: Ptr{Any}
  invokeGraph0 :: Ptr{Any}
  invokeGraph1 :: Ptr{Any}
  invokeGraph2 :: Ptr{Any}
  retain       :: Ptr{Any}
  release      :: Ptr{Any}
  get_type     :: Ptr{Any}
  get_eltype   :: Ptr{Any}
  get_dataptr  :: Ptr{Any}
  get_length   :: Ptr{Any}
  get_ndims    :: Ptr{Any}
  get_dims     :: Ptr{Any}
  get_strides  :: Ptr{Any}
end

@struct type CGraph
  numNodes         :: Int32
  nodeNames        :: Ptr{Ptr{Uint8}}
  numValues        :: Int32
  values           :: Ptr{Float64}
  numRowPtrs       :: Int32
  rowValueOffsets  :: Ptr{Int32}
  collOffsets      :: Ptr{Int32}
end

function packit(struct)
  ret = IOBuffer()
  pack(ret, struct)
  ret
end

function td_init(language; classpath = ".", mainclass = "main", juliapath = "", homedir = ".")
  if language == :java
    env = ccall((:td_env_java, "td"), Ptr{TDEnv}, (Ptr{Uint8}, Ptr{Uint8}, Ptr{Uint8}), homedir, classpath, mainclass)
  elseif language == :R
    env = ccall((:td_env_r, "td"), Ptr{TDEnv}, (Ptr{Uint8},), homedir)
  elseif language == :julia
    env = ccall((:td_env_julia, "td"), Ptr{TDEnv}, (Ptr{Uint8},), homedir)
  elseif language == :python
    env = ccall((:td_env_python, "td"), Ptr{TDEnv}, (Ptr{Uint8},), homedir)
  end
  unsafe_load(env)
end

# -------------------------------------------------------------------------------------------------------------------------
# Delite-based Louvain
# -------------------------------------------------------------------------------------------------------------------------
function dlouvain(java :: TDEnv, csr :: CSR)
  #in_graph   = CGraph(0, String[], 6, Int32[1, 1, 1, 1, 1, 1], 4, Int32[0, 2, 4, 6], Int32[1, 2, 0, 2, 0, 1])
  in_graph   = CGraph(0, pointer(String[]), length(csr.colidx), pointer(ones(Float64, length(csr.colidx))), length(csr.rowptr), pointer(csr.rowptr), pointer(csr.colidx))
  out_graph  = CGraph(0, C_NULL, 0, C_NULL, 0, C_NULL, C_NULL)
  out_packed = packit(out_graph)
  ccall(java.invokeGraph1, Void, (Ptr{Graph}, Ptr{Int8}, Ptr{Graph}), pointer(out_packed.data), "communityDetection", pointer(packit(in_graph).data))
  seek(out_packed, 0)
  cg = unpack(out_packed, CGraph)
  #return CSR(pointer_to_array(cg.nodeNames, cg.numNodes), pointer_to_array(cg.rowValueOffsets, cg.numRowPtrs), pointer_to_array(cg.collOffsets, cg.numValues))
  return CSR([ "" for i = 1:cg.numNodes ], pointer_to_array(cg.rowValueOffsets, cg.numRowPtrs), pointer_to_array(cg.collOffsets, cg.numValues))
end

# -------------------------------------------------------------------------------------------------------------------------
# R-based Truncated SVD Clusterer
# -------------------------------------------------------------------------------------------------------------------------
function rtsvd(R :: TDEnv, csr :: CSR; k = 2)
  global first_rtsvd_call
  in_graph   = CGraph(length(csr.names), pointer(csr.names), length(csr.colidx), pointer(ones(Float64, length(csr.colidx))), length(csr.rowptr), pointer(csr.rowptr), pointer(csr.colidx))
  out_graph  = CGraph(0, C_NULL, 0, C_NULL, 0, C_NULL, C_NULL)
  out_packed = packit(out_graph)
  output     = Array(Int32, 100) # junk
  if first_rtsvd_call
    ccall(R.eval, Void, (Ptr{Void}, Ptr{Uint8}), pointer(output), """source("../cluster/r_fielder/graph_cluster.r")""")
    first_rtsvd_call = false
  end
  ccall(R.invokeGraph2, Void, (Ptr{Graph}, Ptr{Int8}, Ptr{Graph}, Int32), pointer(out_packed.data), "fielder_cluster_and_graph", pointer(packit(in_graph).data), k)
  seek(out_packed, 0)
  cg = unpack(out_packed, CGraph)
  #return CSR(pointer_to_array(cg.nodeNames, cg.numNodes), pointer_to_array(cg.rowValueOffsets, cg.numRowPtrs), pointer_to_array(cg.collOffsets, cg.numValues))
  return CSR([ "" for i = 1:cg.numNodes ], pointer_to_array(cg.rowValueOffsets, cg.numRowPtrs), pointer_to_array(cg.collOffsets, cg.numValues))
end

# -------------------------------------------------------------------------------------------------------------------------
# Main
# -------------------------------------------------------------------------------------------------------------------------
function main()
  # (0) Download files as needed
  getfile("http://data.dws.informatik.uni-mannheim.de/hyperlinkgraph/pld-index.gz", "../data/pld-index.gz"; expected_size = 311068910)
  getfile("http://data.dws.informatik.uni-mannheim.de/hyperlinkgraph/pld-arc.gz", "../data/pld-arc.gz"; expected_size = 2912232966)

  # (1) Read and setup data structures
  g   = Graph("../data/pld-index.gz", "../data/pld-arc.gz")
  CSR(g)
  #adj = sparse(g.sources, g.dests, ones(length(g.sources)))
  
  # (2) call analytics via TD

  # test that the matrix is OK
  #s, u = eigs([ spzeros(size(adj, 1), size(adj, 1)) adj; adj' spzeros(size(adj, 2), size(adj, 2)) ]; nev=2, ritzvec=true)
  
  # test 2 for Jeff: tld cluster stub/mock
  @timer "tld clustering" subg = tldcluster(g)
  println(subg) # should be a fully connected, 4 node graph

  # pca        = @spawn td("r_pca", g)
  # modularity = @spawn td("j_louvain", g)
  # centrality = @spawn td("s_centrality", g)
  
  # (3) call viz via TD
  
  # b = @spawn td("p_bokeh", pca)
  # t = @spawn td("p_tangelo", modularity)
end

function test()
  # (0) Init
  path = String[ "cd.jar", "CommunityDetectionJar-bin.jar", "runtime_2.10.jar", "scala-library-2.10.3.jar", "la4j-0.4.9.jar", "commons-lang3-3.3.2.jar" ] 
  java = td_init(:java, classpath = join(map(s -> "../cluster/dlouvain/$s", path), ":"), mainclass = "CommunityDetectionTest")
  R    = td_init(:R)
  
  # (1) Read and setup data structures
  g   = Graph("../data/pld-index-sample.gz", "../data/pld-arc-sample.gz")
  g.dests = rand(1:20, 30)
  csr = CSR(g)
  println(length(g.sources), " ", g.sources)
  println(length(g.dests), " ", g.dests)
  println(length(csr.rowptr), " ", csr.rowptr)
  println(length(csr.colidx), " ", csr.colidx)
  
  # (2) call analytics via TD

  # test 1 for Stanford: call louvain
  louv = dlouvain(java, csr)

  # test 2 for Jeff: tld cluster stub/mock
  subg = tldcluster(g)
  @info "tld graph == $subg" # should be a fully connected, 4 node graph

  # test 3 for Yale: call R Graph cluster (TSVD)
  tsvd = rtsvd(R, csr)
end

if length(ARGS) > 0 && ARGS[1] == "test"
  test()
else
  main()
end

