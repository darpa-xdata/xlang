using Stage
using GZip

# -------------------------------------------------------------------------------------------------------------------------
# Utilities
# -------------------------------------------------------------------------------------------------------------------------
log = Log(STDERR)

function getfile(url, fn; expected_size = -1)
  if filesize(fn) != expected_size
    @timer log "downloading $fn from $url" download(url, fn)
  else
    @info log "$fn already downloaded, using local version."
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
  sources :: Array{Int64} # source and dests are sorted and can be used to construct a CSC or CSR sparse matrix quickly
  dests   :: Array{Int64}
end

type CSR
  names :: Array{String}
  rowptr :: Array{Int64}
  colidx :: Array{Int64}
end

Graph() = Graph((String)[],
                (Int64)[],
                (Int64)[])

function GraphPre(nodefn, arcfn)
  g = Graph()

  # read the index
  @timer log "reading of node lengths" len = zcountlines(nodefn)
  g.names = Array(String, len)

  @timer log "reading node names" zopen(nodefn) do f
    for (i, line) in enumerate(eachline(f))
      l = split(strip(line), '\t')
      g.names[i] = l[1]
    end
  end
  @info log "read $(length(g.names)) arcs."

  # read arcs
  @timer log "reading arc lengths" len = zcountlines(arcfn)

  g.sources = Array(Int64, len)
  g.dests   = Array(Int64, len)
  @timer log "reading arcs" zopen(arcfn) do f
    for (i, arcl) in enumerate(eachline(f))
      arc          = split(strip(arcl[2]), '\t')
      g.sources[i] = int64(arc[1])
      g.dests[i]   = int64(arc[2])
    end
  end
  @info log "read $(length(g.sources)) arcs."

  return g
end

function Graph(nodefn, arcfn)
  g = Graph()

  # read the index
  @timer log "reading node names" zopen(nodefn) do f
    for l in map(x -> split(strip(x), '\t'), eachline(f))
      push!(g.names, l[1])
    end
  end
  @info log "read $(length(g.names)) nodes."

  # test
  #@timer log "[test] name2tld" tnames = [ name2tld(g.names[i]) for i = 1:1_000_000 ]
  #@info log "tnames = $(length(tnames))"
  #@timer log "[test] splittld" nnames = [ splittld(g.names[i]) for i = 1:1_000_000 ]
  #@info log "tnames = $(length(nnames))"

  # read arcs
  @timer log "reading arc lengths" len = zcountlines(arcfn)

  g.sources = Array(Int64, len)
  g.dests   = Array(Int64, len)
  gc_disable()

  @timer log "reading arcs" zopen(arcfn) do f
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
  @info log "done. read $(length(g.sources)) arcs."

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
  @timer log "names -> tld" tnames = [ name2tld(graph.names[i]) for i = 1:length(graph.names) ]

  # analyze node names
  tldindex = (String=>Int64)[]
  tldnames = (String)[]
  tidx     = 1
  @timer log "making name index" begin
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
  arcset   = [ Set{Int64}() for x = 1:length(tldnames) ]
  arcindex = 1
  @timer log "making arcsets" begin
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
  srcs    = Array(Int64, numarcs) 
  dests   = Array(Int64, numarcs) 
  ai      = 1
  @timer log "final arcs" begin
    for i = 1:length(arcset), j = 1:length(arcset[i])
      srcs[ai]  = i
      dests[ai] = j
      ai       += 1
    end
  end

  return Graph(tldnames, srcs, dests)
end

function CSR(g :: Graph)
  previ = 0
  for i = 1:length(g.sources)
    if g.sources[i] != previ
      if previ > 0
        for j = previ:i
          push!(rowptr, i) #NOTE: empty rows have the same idx as next row
        end
      end
      previ = i
    end
  end
  return CSR(graph.names, rowptr, dests)
end

# -------------------------------------------------------------------------------------------------------------------------
# Main
# -------------------------------------------------------------------------------------------------------------------------
function main()
  # (0) Download files as needed
  getfile("http://data.dws.informatik.uni-mannheim.de/hyperlinkgraph/pld-index.gz", "pld-index.gz"; expected_size = 311068910)
  getfile("http://data.dws.informatik.uni-mannheim.de/hyperlinkgraph/pld-arc.gz", "pld-arc.gz"; expected_size = 2912232966)

  # (1) Read and setup data structures
  g   = Graph("pld-index.gz", "pld-arc.gz")
  CSR(g)
  #adj = sparse(g.sources, g.dests, ones(length(g.sources)))
  
  # (2) call analytics via TD

  # test that the matrix is OK
  #s, u = eigs([ spzeros(size(adj, 1), size(adj, 1)) adj; adj' spzeros(size(adj, 2), size(adj, 2)) ]; nev=2, ritzvec=true)
  
  # test 2 for Jeff: tld cluster stub/mock
  @timer log "tld clustering" subg = tldcluster(g)
  println(subg) # should be a fully connected, 4 node graph

  # pca        = @spawn td("r_pca", g)
  # modularity = @spawn td("j_louvain", g)
  # centrality = @spawn td("s_centrality", g)
  
  # (3) call viz via TD
  
  # b = @spawn td("p_bokeh", pca)
  # t = @spawn td("p_tangelo", modularity)
end

function test()
  # (1) Read and setup data structures
  g   = Graph("pld-index-sample.gz", "pld-arc-sample.gz")
  adj = sparse(g.sources, g.dests, ones(length(g.sources)))
  
  # (2) call analytics via TD

  # test that the matrix is OK
  s, u = eigs([ spzeros(size(adj, 1), size(adj, 1)) adj; adj' spzeros(size(adj, 2), size(adj, 2)) ]; nev=2, ritzvec=true)
  @info log "$(s)"

  # test 2 for Jeff: tld cluster stub/mock
  subg = tldcluster(g)
  @info log "tld graph == $subg" # should be a fully connected, 4 node graph
end

if length(ARGS) > 0 && ARGS[1] == "test"
  test()
else
  main()
end

