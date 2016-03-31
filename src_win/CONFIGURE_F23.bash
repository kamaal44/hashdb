#!/bin/bash
cat <<EOF
*******************************************************************
        Configuring Fedora for cross-compiling multi-threaded
		 64-bit Windows programs with mingw.
*******************************************************************

This script will configure a fresh Fedora system to compile with
mingw64.  Please perform the following steps:

1. Install F20 or newer, running with you as an administrator.
   For a VM:

   1a - download the ISO for the 64-bit DVD (not the live media) from:
        http://fedoraproject.org/en/get-fedora-options#formats
   1b - Create a new VM using this ISO as the boot.
       
2. Run this script to configure the system to cross-compile bulk_extractor.
   Files will be installed and removed from this directory during this process.
   Parts of this script will be run as root using "sudo".

press any key to continue...
EOF
read

MPKGS="autoconf automake gcc gcc-c++ libtool "
MPKGS+="wine wget bison "
MPKGS+="libewf libewf-devel "
MPKGS+="openssl-devel "
MPKGS+="cabextract gettext-devel "
MPKGS+="mingw64-gcc mingw64-gcc-c++ "
#MPKGS+="mingw32-nsis "

if [ ! -r /etc/redhat-release ]; then
  echo This requires Fedora Linux
  exit 1
fi

if grep 'Fedora.release.' /etc/redhat-release ; then
  echo Fedora Release detected
else
  echo This script is only tested for Fedora Release 20 and should work on F20 or newer.
  exit 1
fi

echo Will now try to install 

sudo yum install -y $MPKGS
if [ $? != 0 ]; then
  echo "Could not install some of the packages. Will not proceed."
  exit 1
fi

echo Attempting to install both DLL and static version of all mingw libraries
echo needed for hashdb.
echo At this point we will keep going even if there is an error...
INST=""
# For these install both DLL and static
for lib in zlib bzip2 winpthreads openssl ; do
  INST+=" mingw64-${lib} mingw64-${lib}-static"
done
sudo yum -y install $INST

echo 
echo "Now performing a yum update to update system packages"
sudo yum -y update

MINGW64=x86_64-w64-mingw32

MINGW64_DIR=/usr/$MINGW64/sys-root/mingw

# from here on, exit if any command fails
set -e

if [ ! -r /usr/x86_64-w64-mingw32/sys-root/mingw/lib/$LIB.a ];
  LIBEWF_TAR_GZ=libewf-20140406.tar.gz
  LIBEWF_URL=https://googledrive.com/host/0B3fBvzttpiiSMTdoaVExWWNsRjg/$LIBEWF_TAR_GZ
  echo Building LIBEWF

  if [ ! -r $LIBEWF_TAR_GZ ]; then
    wget --content-disposition $LIBEWF_URL
  fi
  tar xvf $LIBEWF_TAR_GZ

  # get the directory that it unpacked into
  DIR=`tar tf $LIBEWF_TAR_GZ |head -1`

  # build and install LIBEWF
  pushd $DIR
  echo
  echo %%% $LIB mingw64

  # patch libewf/libuna/libuna_inline.h for GCC5
  patch -p0 <../libewf-20140406-gcc5-compatibility.patch

  # configure
  CPPFLAGS=-DHAVE_LOCAL_LIBEWF mingw64-configure --enable-static --disable-shared
  make clean
  make
  sudo make install
  make clean
  popd
  rm -rf $DIR
fi



#
# ICU requires patching and a special build sequence
#

function is_installed {
  LIB=$1
  if [ -r /usr/x86_64-w64-mingw32/sys-root/mingw/lib/$LIB.a ];
  then
    return 0
  else 
    return 1
  fi
}

echo "Building and installing ICU for mingw"
ICUVER=53_1
ICUFILE=icu4c-$ICUVER-src.tgz
ICUDIR=icu
ICUURL=http://download.icu-project.org/files/icu4c/53.1/$ICUFILE

if is_installed libicuuc
then
  echo ICU is already installed
else
  if [ ! -r $ICUFILE ]; then
    wget $ICUURL
  fi
  tar xf $ICUFILE

  # patch ICU for MinGW cross-compilation
  pushd icu
  patch -p0 <../icu4c-53_1-simpler-crossbuild.patch
  patch -p0 <../icu4c-53_1-mingw-w64-mkdir-compatibility.patch
  popd

  ICUDIR=`tar tf $ICUFILE|head -1`

  ICU_DEFINES="-DU_USING_ICU_NAMESPACE=0 -DU_CHARSET_IS_UTF8=1 -DUNISTR_FROM_CHAR_EXPLICIT=explicit -DUNSTR_FROM_STRING_EXPLICIT=explicit"

  ICU_FLAGS="--disable-extras --disable-icuio --disable-layout --disable-samples --disable-tests"

  # build ICU for Linux to get packaging tools used by MinGW builds
  echo
  echo icu linux
  rm -rf icu-linux
  mkdir icu-linux
  pushd icu-linux
  CC=gcc CXX=g++ CFLAGS=-O3 CXXFLAGS=-O3 CPPFLAGS="$ICU_DEFINES" ../icu/source/runConfigureICU Linux --enable-shared $ICU_FLAGS
  make VERBOSE=1
  popd
  
  # build 64-bit ICU for MinGW
  echo
  echo icu mingw64
  rm -rf icu-mingw64
  mkdir icu-mingw64
  pushd icu-mingw64
  eval MINGW=\$MINGW64
  eval MINGW_DIR=\$MINGW64_DIR
  ../icu/source/configure CC=$MINGW-gcc CXX=$MINGW-g++ CFLAGS=-O3 CXXFLAGS=-O3 CPPFLAGS="$ICU_DEFINES" --enable-static --disable-shared --prefix=$MINGW_DIR --host=$MINGW --with-cross-build=`realpath ../icu-linux` $ICU_FLAGS --disable-tools --disable-dyload --with-data-packaging=static
  make VERBOSE=1
  sudo make install
  make clean
  popd
  rm -rf icu-mingw64
  rm -rf $ICUDIR icu-linux
  echo "ICU mingw installation complete."
fi

# build pexports on Linux
if [ ! -r /usr/local/bin/pexports ]; then
  echo "Building and installing pexports"
  rm -rf pexports-0.47 pexports-0.47-mingw32-src.tar.zx
  wget https://sourceforge.net/projects/mingw/files/MinGW/Extension/pexports/pexports-0.47/pexports-0.47-mingw32-src.tar.xz

  tar xf pexports-0.47-mingw32-src.tar.xz

  cd pexports-0.47
  ./configure
  make
  sudo make install
  cd ..
  rm -rf pexports-0.47 pexports-0.47-mingw32-src.tar.zx
fi

#
#
#

echo ================================================================
echo ================================================================
echo 'You are now ready to cross-compile for win64.'
echo 'For example: cd hashdb; mkdir win64; cd win64; mingw64-configure; make'