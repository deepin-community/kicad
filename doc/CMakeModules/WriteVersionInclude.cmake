#
# Part of the KiCad Documentation Project
#
# (c) KiCad Developers

set( GIT_VERSION_TEXT
":kicad-version-long: ${KICAD_VERSION_LONG}
:kicad-version-short: ${KICAD_VERSION_SHORT}
:doc-commit: ${DOC_COMMIT}
" )

set( _write_version_file ON )
if( EXISTS ${OUTPUT_FILE} )
    file( READ ${OUTPUT_FILE} _old_contents )
    if( _old_contents STREQUAL GIT_VERSION_TEXT )
        message( STATUS "Not updating ${OUTPUT_FILE}" )
        set( _write_version_file OFF )
    endif()
endif()

if( _write_version_file )
    message( STATUS "Writing ${OUTPUT_FILE} with versions: ${DOC_COMMIT}, ${KICAD_VERSION_LONG}. ${KICAD_VERSION_SHORT}" )
    file( WRITE ${OUTPUT_FILE} ${GIT_VERSION_TEXT} )
endif()

