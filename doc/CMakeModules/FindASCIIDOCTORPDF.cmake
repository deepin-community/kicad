#
# (c)2015 KiCad-Developers
# (c)2015 Brian Sidebotham
#
# Part of the KiCad project
#

set( _DBG OFF )

find_program( ASCIIDOCTORPDF_PROGRAM asciidoctor-web-pdf )

if( ASCIIDOCTORPDF_PROGRAM )
    execute_process(
        COMMAND asciidoctor-web-pdf --version
        RESULT_VARIABLE _ADOCPDF_EXE_RESULT
        OUTPUT_VARIABLE _ADOCPDF_EXE_OUTPUT
        ERROR_VARIABLE _ADOCPDF_EXE_ERROR )

    if( _DBG )
        message( STATUS "ASCIIDOCTORPDF result: ${_ADOCPDF_EXE_RESULT}" )
        message( STATUS "ASCIIDOCTORPDF output: ${_ADOCPDF_EXE_OUTPUT}" )
    endif()

    if ( _ADOCPDF_EXE_RESULT MATCHES 0 )
        set( ASCIIDOCTORPDF_COMMAND "${ASCIIDOCTORPDF_PROGRAM}" )
    else()
        # Attempt to see if a bat file can be run which invokes ASCIIDOCTORPDF
        execute_process( COMMAND asciidoctor-pdf.bat --version
            RESULT_VARIABLE _ADOCPDF_EXE_RESULT
            OUTPUT_VARIABLE _ADOCPDF_EXE_OUTPUT
            ERROR_VARIABLE _ADOCPDF_EXE_ERROR )

            set( ASCIIDOCTORPDF_COMMAND "asciidoctor-pdf.bat" )
    endif()
endif()

if( ASCIIDOCTORPDF_COMMAND )
    # Get the ASCIIDOCTORPDF version number
    string( REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" ADOCPDF_VERSION "${_ADOCPDF_EXE_OUTPUT}" )
endif()

# Generate the *_FOUND as necessary, etc.
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args(
    ASCIIDOCTORPDF
    REQUIRED_VARS ASCIIDOCTORPDF_COMMAND
    VERSION_VAR ADOCPDF_VERSION )
