#!/bin/bash

## =================================================================
## PACKAGE INSTALLATION
##
## This script will install all the packages that are needed to
## build and run the DBMS.
##
## Supported environments:
##  * Ubuntu 22.04 (x86-64)
##  * macOS 11 Big Sur (x86-64 or ARM)
##  * macOS 12 Monterey (x86-64 or ARM)
## =================================================================

main() {

	set -o errexit

	checkSystem

	if [ "$1" == "-y" ]; then
		install
	else
		echo "PACKAGES WILL BE INSTALLED. THIS MAY BREAK YOUR EXISTING TOOLCHAIN."
		echo "YOU ACCEPT ALL RESPONSIBILITY BY PROCEEDING."
		read -p "Proceed? [y/N] : " yn

		case $yn in
		Y | y) install ;;
		*) ;;
		esac
	fi
  git submodule init
  git submodule update
  cd third_part/benchmark && cmake -E make_directory "build" && cmake -E chdir "build" cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DCMAKE_BUILD_TYPE=Release ../ && cmake --build "build" --config Release --target install
	echo "Script complete."
}

checkSystem() {
	os_version="unknown"
	if [[ -n $(find /etc -name "redhat-release") ]] || grep </proc/version -q -i "centos"; then
		# 检测系统版本号
		centosVersion=$(rpm -q centos-release | awk -F "[-]" '{print $3}' | awk -F "[.]" '{print $1}')
		if [[ -z "${centosVersion}" ]] && grep </etc/centos-release "release 8"; then
			centosVersion=8
		fi
		os_version="centos"

	elif grep </etc/issue -q -i "debian" && [[ -f "/etc/issue" ]] || grep </etc/issue -q -i "debian" && [[ -f "/proc/version" ]]; then
		if grep </etc/issue -i "8"; then
			debian_version=8
		fi
		os_version="debian"

	elif grep </etc/issue -q -i "ubuntu" && [[ -f "/etc/issue" ]] || grep </etc/issue -q -i "ubuntu" && [[ -f "/proc/version" ]]; then
		os_version="ubuntu"
	fi
}

install() {

	set -x
	UNAME=$(uname | tr "[:lower:]" "[:upper:]")

	case $UNAME in
	DARWIN) install_mac ;;

	LINUX)

		if [ "${os_version}" == "unknown" ]; then
			echo -e "You are using ${os_version} operating system, which is currently not supported."
			exit 1
		else
			echo -e "You are using ${os_version} operating system."
		fi

		if [ "${os_version}" == "ubuntu" ]; then

			version=$(cat /etc/os-release | grep VERSION_ID | cut -d '"' -f 2)
			case $version in
			18.04) install_ubuntu ;;
			20.04) install_ubuntu ;;
			22.04) install_ubuntu ;;
			*) give_up ;;
			esac
		elif [ "${os_version}" == "centos" ]; then
      install_centos
		fi
		;;

	*) give_up ;;
	esac
}

give_up() {
	set +x
	echo "Unsupported distribution '${os_version}'"
	echo "Please contact our support team for additional help."
	echo "Be sure to include the contents of this message."
	echo "Platform: $(uname -a)"
	echo
	echo "https://github.com/qingfuliu/DeftRPC"
	echo
	exit 1
}

install_mac() {
	# Install Homebrew.
	if test ! $(which brew); then
		echo "Installing Homebrew (https://brew.sh/)"
		bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
	fi
	# Update Homebrew.
	brew update
	# Install packages.
	brew ls --versions cmake || brew install cmake
	brew ls --versions coreutils || brew install coreutils
	brew ls --versions doxygen || brew install doxygen
	brew ls --versions git || brew install git
	(brew ls --versions llvm | grep 14) || brew install llvm@14
	brew ls --versions libelf || brew install libelf
}

install_centos() {

  yum update -y
	# Install packages.
	yum -y install \
		clang \
		git-clang-format \
		cmake \
		git \
		pkg-config
}

install_ubuntu() {

	# Update apt-get.
	apt-get -y update
	# Install packages.
	apt-get -y install \
		build-essential \
		clang \
		clang-format \
		clang-tidy \
		cmake \
		git \
		pkg-config
}

main "$@"
