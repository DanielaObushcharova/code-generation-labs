FROM archlinux:latest
RUN pacman --noconfirm -Syu gcc flex llvm make go
RUN mkdir /project
WORKDIR /project
CMD sleep inf
