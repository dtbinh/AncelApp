add_library( datamgmt datareader.cpp datareaderann.cpp datareaderbvh.cpp datareaderbvhquat.cpp joint.cpp supplementary.cpp supplementaryann.cpp supplementarybvh.cpp )
target_link_libraries( datamgmt )
