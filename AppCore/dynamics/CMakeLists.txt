add_library( dynamics transitionreward.cpp dynamics.cpp dynamicsacc.cpp dynamicsgp.cpp dynamicsgpacc.cpp dynamicsgpact.cpp dynamicstimegp.cpp )
target_link_libraries( dynamics )
