Name:       capi-media-metadata-editor
Summary:    A metadata editor library in Tizen Native API
Version: 0.1.5
Release:    0
Group:      Multimedia/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(taglib)
BuildRequires:  pkgconfig(aul)
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
The media metadata editing library in Tizen Native API package.

%package devel
Summary:  A metadata editor library in Tizen Native API (Development)
Group:    Multimedia/Development
Requires: %{name} = %{version}-%{release}

%description devel
The media metadata editing library in Tizen Native API package. (Development files included)

%prep
%setup -q

%build
export CFLAGS+=" -Wextra -Wno-array-bounds"
export CFLAGS+=" -Wno-ignored-qualifiers -Wno-unused-parameter -Wshadow"
export CFLAGS+=" -Wwrite-strings -Wswitch-default"
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/%{_datadir}/license
cp -rf %{_builddir}/%{name}-%{version}/LICENSE.APLv2.0 %{buildroot}/%{_datadir}/license/%{name}

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%manifest capi-media-metadata-editor.manifest
%{_libdir}/libcapi-media-metadata-editor.so
%{_datadir}/license/%{name}

%files devel
%{_includedir}/metadata-editor/*.h
%{_libdir}/pkgconfig/capi-media-metadata-editor.pc
