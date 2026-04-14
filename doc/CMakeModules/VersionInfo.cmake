# These variables set version information that is used in the documentation. They are used to derive various
# information about branch (master/stable), major/minor/patch versions, etc.
# DOC_COMMIT is intended to be set externally (in CI) with the current
# documentation commit hash, but it defaults to a sensible value.
# DOC_VERSION needs to be updated manually when the docs are tagged for a new stable version (e.g. 9.0.2) or
# when a new stable version branches off of the master branch (e.g. 10.0.0 and 10.99.0).
set( DOC_VERSION "10.0.0" CACHE STRING "Version info for docs/KiCad application in Major.Minor.Patch format." )
set( DOC_COMMIT "${DOC_VERSION}" CACHE STRING "Exact documentation revision. Defaults to the value of DOC_VERSION, but is intended to be externally set to the documention commit hash." )
