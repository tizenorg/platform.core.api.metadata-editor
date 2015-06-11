Name:       capi-media-metadata-editor
Summary:    A metadata editor library in SLP C API
Version: 0.1.1
Release:    0
Group:      System/Libraries
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
A metadata editor library in SLP C API

%package devel
Summary:  A metadata editor library in SLP C API (Development)
Group:    TO_BE/FILLED_IN
Requires: %{name} = %{version}-%{release}

%description devel
A metadata editor library in SLP C API

%prep
%setup -q

%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
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
