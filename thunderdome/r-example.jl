using StrPack

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
  numEdges         :: Int32
  nodeNames        :: Ptr{Ptr{Uint8}}
  values           :: Ptr{Float64}
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

function td_graph(env :: TDEnv, fun, k)
  in_graph   = CGraph(3, 6, String["1", "2", "3"], Float64[1, 1, 1, 1, 1, 1], Int32[0, 2, 4, 6], Int32[1, 2, 0, 2, 0, 1])
  out_graph  = CGraph(0, 0, C_NULL, C_NULL, C_NULL, C_NULL)
  out_packed = packit(out_graph)
  ccall(env.invokeGraph2, Void, (Ptr{CGraph}, Ptr{Int8}, Ptr{CGraph}, Int32), out_packed.data, fun, packit(in_graph).data, k)
  seek(out_packed, 0)
  return unpack(out_packed, CGraph)
end

function test()
  env = td_init(:R)
  output = Array(Int32, 100)
  ccall(env.eval, Void, (Ptr{Void}, Ptr{Uint8}), pointer(output), """source("../webgraph/cluster/r_fielder/graph_cluster.r")""") # prep
  out_graph = td_graph(env, "fielder_cluster_and_graph", 2)
end

# test
res = test()
