#*******************************************************************************
#*   Copyright (C)      Mathieu Goulin <mathieu.goulin@gadz.org>               *
#*                                                                             *
#*   This program is free software; you can redistribute it and/or             *
#*   modify it under the terms of the GNU Lesser General Public                *
#*   License as published by the Free Software Foundation; either              *
#*   version 2.1 of the License, or (at your option) any later version.        *
#*                                                                             *
#*   The GNU C Library is distributed in the hope that it will be useful,      *
#*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
#*   Lesser General Public License for more details.                           *
#*                                                                             *
#*   You should have received a copy of the GNU Lesser General Public          *
#*   License along with the GNU C Library; if not, write to the Free           *
#*   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA         *
#*   02111-1307 USA.                                                           *
#*******************************************************************************
# To use glibc, libld and curl i chose the debian base
FROM debian:latest

# Me
MAINTAINER mathieu.goulin@gadz.org

# Install curl
RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get -yq install -y curl \
    && rm -rf /var/lib/apt/lists/*

# Add the package to debian
ADD filenotify.tar.gz /

# Configure to rebase plugin path
RUN sed -i "s@^plugins_dir=.*@plugins_dir=/usr/lib/filenotify/@" /etc/filenotify/filenotify.config

# Entrypoint
ENTRYPOINT ["/usr/bin/filenotify", "-c", "/etc/filenotify/filenotify.config"]
