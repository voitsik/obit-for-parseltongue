/* SWIG interface file for ObitVersion.c */

%{
extern const char *ObitVersion();
%}

%inline %{
extern const char *Version() {
    return ObitVersion();
}
%}
