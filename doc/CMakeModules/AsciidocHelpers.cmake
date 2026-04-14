#
# Part of the KiCad ASCIIDOC Documentation Project
#
# (c)2015 KiCad Developers
# (c)2015 Brian Sidebotham <brian.sidebotham@gmail.com>
#

# Variables used to keep lists of program arguments
set( ASCIIDOC_OPTIONS "" )

# Add an asciidoc to PDF conversion target
macro( add_adoc_pdf_target TARGET INFILE OUTFILE LANGUAGE )
    if( "${PDF_GENERATOR}" MATCHES "ASCIIDOCTORPDF" )
        set( ASCIIDOCTORPDF_OPTIONS "" )
        # have to clean up temporary file created by asciidoctor-web-pdf
        string( REGEX REPLACE "[.]adoc$" ".html" ASCIIDOCTORPDF_TEMP_FILE ${INFILE} )
        add_custom_target( ${TARGET} DEPENDS ${OUTFILE} )
        add_custom_command( OUTPUT ${OUTFILE}
                            COMMAND ${ASCIIDOCTORPDF_COMMAND} ${ASCIIDOCTORPDF_OPTIONS} ${LANGUAGE_OPTIONS} -o ${OUTFILE} ${INFILE}
                            DEPENDS ${INFILE} )
        add_custom_command( TARGET ${TARGET} POST_BUILD
                            COMMAND cmake -E remove ${ASCIIDOCTORPDF_TEMP_FILE} )
    else()
        message( FATAL_ERROR "ASCIIDOCTORPDF is the only supported PDF_GENERATOR. Exiting." )
    endif()
endmacro()

# Add an asciidoc to HTML conversion target
macro( add_adoc_html_target TARGET INFILE OUTFILE LANGUAGE )
    add_custom_target( ${TARGET} DEPENDS ${OUTFILE} )
    add_custom_command( OUTPUT ${OUTFILE}
                        COMMAND ${ASCIIDOC_COMMAND} ${ASCIIDOC_OPTIONS} ${LANGUAGE_OPTIONS} -o ${OUTFILE} ${INFILE}
                        DEPENDS ${INFILE} )
endmacro()

# Pass an option to asciidoc
macro( add_adoc_option )
    foreach( OPT ${ARGN} )
        list( APPEND ASCIIDOC_OPTIONS ${OPT} )
    endforeach()
endmacro()
