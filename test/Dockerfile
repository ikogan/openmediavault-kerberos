FROM ikogan/openmediavault:dev

MAINTAINER Ilya Kogan <ikogan@flarecode.com>

ENV DEBIAN_FRONTEND noninteractive

# Enable the omvextras testing repo, the docker repo, and the docker testing repo
RUN omv-config -m //system/omvextrasorg/testing "1"; omv-config -m //system/omvextrasorg/docker "1"; omv-config -m //system/omvextrasorg/dockertesting "1"

# Update omv-extras
RUN omv-mkconf omvextrasorg