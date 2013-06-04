
# compile lua to bytecode

%.lbc: %.lua
	$(LUAC) -o $@ $<

# turn lua bytecode into a linkable object

%.lbc.ld.o: %.lbc
	$(LD) -r -b binary $< -o $@

# ld defaults to .data. want .rodata.

%.lbc.o: %.lbc.ld.o
	$(OBJCOPY) --rename-section \
		.data=.rodata,alloc,load,readonly,data,contents \
		$< $@
