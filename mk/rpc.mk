
RPCGEN = rpcgen

#
# glibc rpcgen quirks:
#
# 1. C sources #include '$(srcdir)/%.h'. should be basename only
#    (i.e. builddir). piping through stdin isn't optimal, but prevents
#    that.
#
# 2. fails to overwrite on rebuild, so rm-f
# 

%.h: %.x
	rm -f $@; $(RPCGEN) -Mh -o $@ < $<

%_xdr.c: %.x %.h
	rm -f $@; $(RPCGEN) -Mc -i 2 -o $@ < $<
