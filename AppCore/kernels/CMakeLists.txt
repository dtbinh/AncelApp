add_library( kernels biaskernel.cpp compoundkernel.cpp distkernel.cpp kernel.cpp linkernel.cpp mlpkernel.cpp rbfkernel.cpp tensorkernel.cpp whitekernel.cpp )
target_link_libraries( kernels )
