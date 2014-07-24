library(Matrix)
library(foreach)
library(itertools)
library(doMC)
library(Rcpp)
library(IRL)
library(SparseM)

#sourceCpp("make_r_sm.cpp")
#registerDoMC()
registerDoSEQ()

setMethod("%*%", signature(x="dgRMatrix", y="dgeMatrix"),
  function(x, y) {
    if (ncol(x) != nrow(y))
      stop("Non-conformable matrix dimensions")
    ret <- foreach(i=1:nrow(x), .combine=rbind) %dopar% {
      foreach(j=1:ncol(y), .combine=cbind) %do% {
        sum(x[i,] * y[,j])
      }
    }
    colnames(ret) <- NULL
    ret
  }
)

clusters_from_proj_nodes <- function(proj_nodes) {
  temp <- apply(proj_nodes, 1, 
              function(x) sum(as.numeric(x > 0)*2^(1:ncol(proj_nodes))))
  us <- unique(sort(temp))
  ret <- as.integer(temp)
  for (i in 1:length(us))
    ret[temp == us[i]] <- as.integer(i)
  ret
}

# Return the elements of the kth cluster
fielder_cluster <- function(m, k, use_irlba=TRUE) {
  if (use_irlba) {
    fit <- irlba(m, nu=k, nv=k)
  } else {
    fit <- svd(m)
  }
  proj_nodes <- (m %*% fit$v)
  clusters_from_proj_nodes(proj_nodes)
}

make_fielder_graph <- function(m, clusters) {
  unique_clusters <- unique(clusters)
  ret_m <- Matrix(0, nrow=length(unique_clusters), ncol=length(unique_clusters))
  for (i in 1:(length(unique_clusters)-1)) {
    i_indices <- which(i == clusters)
    for (j in 2:length(unique_clusters)) {
      j_indices <- which(j == clusters)
      if (any(m[i_indices, j_indices] > 0)) {
        ret_m[i, j] <- 1
        ret_m[j, i] <- 1
      }
    }
  }
  ret_m
}

fielder_cluster_and_graph <- function(m, k) {
  num_singular_vectors <- floor(log2(k))
  print(num_singular_vectors)
  if (num_singular_vectors != log2(k)) {
    cat(paste("Warning: The k parameter should be a power of 2.\n",
              "The number of singular vectors is being set to ",
              num_singular_vectors, ".\nThe number of clusters is being set",
              "to a maximum of ", 2^num_singular_vectors, "\n", sep=""))
  }
  clusters <- fielder_cluster(m, num_singular_vectors)
  # Note that the following line could be made *way* more efficient.
  a <- as(make_fielder_graph(m, clusters), "matrix.csc")
  b <- as(a, "Matrix")
  c <- as(b, "matrix.csr")
  graph <- as(c, "dgRMatrix")
#  graph <- as(as(make_fielder_graph(m, clusters), "matrix.csr"), "dgRMatrix")
  ret_graph <- list(numNodes=as.integer(nrow(graph)),
                    numValues=as.integer(length(graph@x)), 
                    values=as.double(graph@x),
                    numRowPtrs=as.integer(length(graph@p)),
                    rowValueOffsets=as.integer(graph@p),
                    colOffsets=as.integer(graph@j))
  list(clusters=clusters, graph=ret_graph)
}

#make_test_graph <- function() {
#  m <- Matrix(nrow=6, ncol=6)
#}

#sourceCpp("r_env_example.cpp")
#m <- make_csr_matrix()
#clusters <- fielder_cluster(m, 2)
#fielder_cluster_and_subgraph(m, clusters)
