FROM registry.gitlab.com/kicad/kicad-ci/doc_containers:latest AS doc-build-env

ARG DOC_COMMIT="commit-default"
ENV DOC_COMMIT="${DOC_COMMIT}"

WORKDIR /src
COPY . .

RUN mkdir -p build
WORKDIR /src/build
RUN cmake -DBUILD_FORMATS="html;pdf" -DDOC_COMMIT=${DOC_COMMIT} ../
RUN make
RUN cpack

FROM busybox AS b
FROM scratch AS output-image

COPY --from=doc-build-env /src/build/src /src
COPY --from=doc-build-env /src/build/*.tar.gz /archive/

# So that one can actually run the image in order to copy files out
# without the context of another docker job running
COPY --from=b /bin/ls /bin/ls
ENTRYPOINT ["/bin/ls"]
