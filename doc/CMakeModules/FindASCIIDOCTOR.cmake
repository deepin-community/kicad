#
# (c)2015 KiCad-Developers
# (c)2015 Brian Sidebotham
#
# Part of the KiCad project
#

set( _DBG OFF )

find_program( ASCIIDOCTOR_PROGRAM asciidoctor )

if( ASCIIDOCTOR_PROGRAM )
    execute_process(
        COMMAND ${ASCIIDOCTOR_PROGRAM} --version
        RESULT_VARIABLE _ADOC_EXE_RESULT
        OUTPUT_VARIABLE _ADOC_EXE_OUTPUT
        ERROR_VARIABLE _ADOC_EXE_ERROR )

    if( _DBG )
        message( STATUS "ASCIIDOCTOR result: ${_ADOC_EXE_RESULT}" )
        message( STATUS "ASCIIDOCTOR output: ${_ADOC_EXE_OUTPUT}" )
    endif()

    if( _ADOC_EXE_RESULT MATCHES 0 )
        set( ASCIIDOC_COMMAND "${ASCIIDOCTOR_PROGRAM}" )
    else()
        # Attempt to see if a bat file can be run which invokes ASCIIDOCTOR
        execute_process(
            COMMAND asciidoctor.bat --version
                RESULT_VARIABLE _ADOC_EXE_RESULT
                OUTPUT_VARIABLE _ADOC_EXE_OUTPUT
                ERROR_VARIABLE _ADOC_EXE_ERROR )

        set( ASCIIDOC_COMMAND "asciidoctor.bat" )
    endif()
endif()

if( ASCIIDOC_COMMAND )
    # Get the ASCIIDOCTOR version number...
    string( REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" ADOC_VERSION "${_ADOC_EXE_OUTPUT}" )
endif()

# Generate the *_FOUND as necessary, etc.
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args(
    ASCIIDOCTOR
    REQUIRED_VARS ASCIIDOC_COMMAND
    VERSION_VAR ADOC_VERSION )
