add_library( dataexport matfile.cpp matreader.cpp matstruct.cpp matwriter.cpp )
target_link_libraries( dataexport application )
