#
# Part of the KiCad ASCIIDOC Documentation Project
#
# (c)2015 KiCad Developers
# (c)2015 Brian Sidebotham <brian.sidebotham@gmail.com>
#

# Require CMake V3.0 as a minimum. You can experiment with setting this lower if that's all you
# have available, but it's not guaranteed to work or be supported.
cmake_minimum_required( VERSION 3.0 )

project( KiCadDocumentation NONE )

set( ADOC_TOOLCHAIN "ASCIIDOCTOR" CACHE STRING "Specify which AsciiDoc toolchain to use: ASCIIDOC|ASCIIDOCTOR" )

# Provide an option for deciding which PDF generation scheme to use
# Desired targets are DBLATEX,FOP and ASCIIDOCTOR-PDF
set( PDF_GENERATOR "ASCIIDOCTORPDF" CACHE STRING "Specify which PDF generator to use: DBLATEX|FOP|ASCIIDOCTORPDF" )

# Provide an option to limit the set of languages built. This helps developers to speed up a
# build they're working on, or limit the number of dependencies for packagers
set( LANGUAGES "" CACHE STRING "Specify languages to build: ca;de;en;es;fr;id;it;ja;nl;pl;ru;zh" )

# Provide an option to decide which documentation formats to build.
set( BUILD_FORMATS "html;pdf" CACHE STRING "Specify which output formats to build html/pdf" )

# Add our custom modules to the module path
set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules" ${CMAKE_MODULE_PATH} )

# Include the Asciidoc target helpers
include( AsciidocHelpers )

# Add the common asciidoc options
add_adoc_option( -b html5 )
add_adoc_option( -a toc2 )
add_adoc_option( --section-numbers )
if( NOT "${ADOC_TOOLCHAIN}" MATCHES "ASCIIDOCTOR" )
    # --theme parameter not supported by asciidoctor
    add_adoc_option( --theme flask )
endif()

# Add the common a2x options
add_a2x_option( --xsltproc-opts=--nonet )
add_a2x_option( --no-xmllint )
add_a2x_option( -f pdf )
#add_a2x_option( --verbose )

if( "${BUILD_FORMATS}" MATCHES "(p|P)(d|D)(f|F)" )

    # Add a2x options depending on which PDF generator we're using
    if( ${PDF_GENERATOR} MATCHES DBLATEX )
        set_dblatex_common_options()
        find_package( DBLATEX REQUIRED )
    endif()

    if( ${PDF_GENERATOR} MATCHES FOP )
        add_a2x_option( --fop )
        add_a2x_option( --xsl-file ${PROJECT_SOURCE_DIR}/xsl/kicad.xsl )
        find_package( FOP REQUIRED )
    endif()

    if( ${PDF_GENERATOR} MATCHES ASCIIDOCTORPDF )
        find_package( ASCIIDOCTORPDF REQUIRED )
    endif()

endif()

# Discover if the PO4A package is available
find_package( PO4A REQUIRED )

# Discover if an Asciidoc toolchain is available
if( ${ADOC_TOOLCHAIN} MATCHES ASCIIDOCTOR )
    find_package( ASCIIDOCTOR REQUIRED )
else()
    find_package( ASCIIDOC REQUIRED )
    find_package( A2X REQUIRED )
endif()

include( KiCadDocumentation )
add_subdirectory( src )

# Start with the package version being unknown, then prove otherwise
set( CPACK_PACKAGE_VERSION "unknown" )

# Use Git to determine parts of the package name
find_package( Git )

if( GIT_FOUND )

    # Discover information about the build (AT CONFIGURE TIME, not as a target that updates in
    # subsequent builds)
    if (UNIX)
        # Linux or macOS
        execute_process(
            COMMAND sh -c "'${GIT_EXECUTABLE}' tag --contains HEAD | sort -V | tail -n 1"
            OUTPUT_VARIABLE git_tags_output
            RESULT_VARIABLE git_tags_result
            ERROR_VARIABLE git_tags_error
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
        )
    elseif (WIN32)
        # Windows
        execute_process(
            COMMAND cmd /C "'${GIT_EXECUTABLE}' tag --contains HEAD | sort /R /Version | set /P= & echo."
            OUTPUT_VARIABLE git_tags_output
            RESULT_VARIABLE git_tags_result
            ERROR_VARIABLE git_tags_error
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
        )
    endif()

    execute_process( COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE git_branch_output
        RESULT_VARIABLE git_branch_result
        ERROR_VARIABLE git_branch_error
        OUTPUT_STRIP_TRAILING_WHITESPACE )

    # Due to how tags work in Git it's necessary for us to ensure a few things. For a start, if we
    # create a new branch, switch to it and create a tag, that tag becomes visible in the master
    # branch. Make sure if we're on the master branch that we use that in the
    if( "${git_branch_output}" STREQUAL "master" )
        # TODO: Should we include a 6-digit hash here too?
        set( CPACK_PACKAGE_VERSION "HEAD" )
    else()
        if( NOT "${git_tags_output}" STREQUAL "" )
            set( CPACK_PACKAGE_VERSION ${git_tags_output} )
        endif()
    endif()
    message (STATUS "Setting package version to ${CPACK_PACKAGE_VERSION}")
endif()

# If there is more than one build format, do not bother transforming the
# package name, otherwise if there's only one, add it to the package name
list( LENGTH BUILD_FORMATS BUILD_FORMATS_LENGTH )
if( ${BUILD_FORMATS_LENGTH} GREATER 1 )
    set( PACKAGE_BUILD_FORMAT "" )
else()
    set( PACKAGE_BUILD_FORMAT "-${BUILD_FORMATS}" )
endif()

# Generate both .tar.gz package target
set( CPACK_GENERATOR "TGZ" )
set( CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON )
set( CPACK_PACKAGE_FILE_NAME "kicad-doc${PACKAGE_BUILD_FORMAT}-${CPACK_PACKAGE_VERSION}" )
include( CPack )
