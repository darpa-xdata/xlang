library(Matrix)
library(foreach)
library(itertools)
library(doMC)
library(Rcpp)
library(irlba)

sourceCpp("make_r_sm.cpp")
registerDoMC()

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

m <- make_csr_matrix()

clusters_from_proj_nodes <- function(proj_nodes) {
  temp <- apply(proj_nodes, 1, 
              function(x) sum(as.numeric(x > 0)*2^(1:ncol(proj_nodes))))
  us <- unique(sort(temp))
  ret <- temp
  for (i in 1:length(us))
    ret[temp == us[i]] <- i
  ret
}

# Return the elements of the kth cluster
fielder_cluster <- function(m, k) {
  irlb_fit <- irlba(m, nu=k, nv=k)
  proj_nodes <- (m %*% irlb_fit$v)
  clusters_from_proj_nodes(proj_nodes)
}

fielder_cluster(a, 2)
