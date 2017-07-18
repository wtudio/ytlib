#---------------------------------------------------------------------
project( ytCommon )
#aux_source_directory( . SourceFiles)

#---------------------------------------------------------------------
# Construct source groups for nice visualisation in Visual Studio.
File(GLOB SourceFiles *.cpp )
source_group( "SourceFiles" FILES ${SourceFiles} )
File(GLOB HeaderFiles *.h )
source_group( "HeaderFiles" FILES ${HeaderFiles} )
#---------------------------------------------------------------------
# Create the elxCommon library.

add_library( ytCommon
  ${SourceFiles}
  ${HeaderFiles}
)
