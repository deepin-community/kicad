FROM registry.gitlab.com/kicad/kicad-ci/doc_containers:latest as doc-build-env

WORKDIR /src
COPY . .

RUN mkdir -p build
WORKDIR /src/build
RUN cmake -DBUILD_FORMATS="html;pdf" ../
RUN make
RUN cpack

FROM busybox as b
FROM scratch as output-image

COPY --from=doc-build-env /src/build/src /src
COPY --from=doc-build-env /src/build/*.tar.gz /archive/

# So that one can actually run the image in order to copy files out
# without the context of another docker job running
COPY --from=b /bin/ls /bin/ls
ENTRYPOINT ["/bin/ls"]
