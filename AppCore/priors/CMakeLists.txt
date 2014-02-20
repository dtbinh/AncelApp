add_library( priors gaussprior.cpp logprior.cpp noprior.cpp prior.cpp )
target_link_libraries( priors )
