require(IRL, quietly=TRUE)
require(Matrix, quietly=TRUE)
require(SparseM, quietly=TRUE, warn.conflicts=FALSE)
require(igraph, quietly=TRUE)

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

dgRMatrix_to_list <- function(m){
  if (class(m) != "dgRMatrix")
    m <- as(as(m, "matrix.csr"), "dgRMatrix")
  list(numNodes=as.integer(nrow(m)),
       numEdges=as.integer(length(m@x)), 
       edgeValues=as.double(m@x),
       rowOffsets=as.integer(m@p),
       colIndices=as.integer(m@j))
}

fielder_cluster_and_graph <- function(m, k, include_matrix=FALSE) {
  num_singular_vectors <- floor(log2(k))
  if (num_singular_vectors != log2(k)) {
    cat(paste("Warning: The k parameter should be a power of 2.\n",
              "The number of singular vectors is being set to ",
              num_singular_vectors, ".\nThe number of clusters is being set",
              "to a maximum of ", 2^num_singular_vectors, "\n", sep=""))
  }
  m <- as(m, "matrix.csr")
  m <- as(m, "dgCMatrix")
  clusters <- fielder_cluster(m, num_singular_vectors)
  # Note that the following line could be made *way* more efficient.
  graph <- as(make_fielder_graph(m, clusters), "matrix.csc")
  graph <- as(graph, "Matrix")
  # Get rid of the self loops.
  diag(graph) <- 0
  ret_graph <- dgRMatrix_to_list(graph)
  ret <- list(clusters=clusters, graph=ret_graph)
  if (include_matrix)
    ret$matrix <- graph
  ret
}

make_test_graph <- function() {
  m <- new("dgRMatrix", Dim=as.integer(c(6, 6)), p=as.integer(rep(0, 7)))
  m[1, 5] <- 1
  m[5, 1] <- 1
  m[2, c(1, 6)] <- 1
  m[c(1, 6), 2] <- 1
  m[3, c(2, 4)] <- 1
  m[c(2, 4), 3] <- 1
  as(as(m, "matrix.csr"), "dgRMatrix")
}

write_adjacency_matrix <- function(m, output_png_file_name) {
  g <- graph.adjacency(m, mode="undirected")
  png(output_png_file_name)
  plot(g)
  dev.off()
}

import_snap_graph <- function(fn, symmetrize=TRUE) {
  x <- read.table(fn, skip=5, header=FALSE)
  # Get rid of the diagonals
  x <- x[x[,1] != x[,2],]
  nr <- max(c(x[,1], x[,2]))+1
  m <- sparseMatrix(i=x[,1]+1, j=x[,2]+1, x=rep(1, length(x[,1])), 
                    dims=c(nr, nr))
  if (symmetrize) {
    m <- m + t(m)
    m@x[m@x == 2] <- 1
  }
  dimnames(m) <- c(list(as.character(1:nrow(m))), list(as.character(1:nrow(m))))
  as(as(m, "matrix.csr"), "dgRMatrix")
}

