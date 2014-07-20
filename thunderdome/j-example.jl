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
    env = ccall((:td_r_init, "td"), Ptr{TDEnv}, (Ptr{Uint8},), homedir)
  elseif language == :julia
    env = ccall((:td_env_julia, "td"), Ptr{TDEnv}, (Ptr{Uint8},), homedir)
  elseif language == :python
    env = ccall((:td_env_python, "td"), Ptr{TDEnv}, (Ptr{Uint8},), homedir)
  end
  unsafe_load(env)
end

function td_graph(env :: TDEnv, fun, in)
  in_graph   = CGraph(0, String[], 6, Int32[1, 1, 1, 1, 1, 1], 4, Int32[0, 2, 4, 6], Int32[1, 2, 0, 2, 0, 1])
  out_graph  = CGraph(0, C_NULL, 0, C_NULL, 0, C_NULL, C_NULL)
  out_packed = packit(out_graph)
  ccall(env.invokeGraph1, Void, (Ptr{Graph}, Ptr{Int8}, Ptr{Graph}), out_packed.data, fun, packit(in_graph).data)
  seek(out_packed, 0)
  return unpack(out_packed, Graph)
end

function test(in_graph)
  env = td_init(:java, classpath = "lib/CommunityDetectionJar-bin.jar:lib/runtime_2.10.jar:lib/scala-library-2.10.3.jar:out:lib/la4j-0.4.9.jar:lib/commons-lang3-3.3.2.jar", 
                mainclass = "CommunityDetectionTest")
  out_graph = td_graph(env, "communityDetection", in_graph)
end

# test
res = test(g)
