local ffi = require('ffi');

ffi.cdef[[
	int printf(const char *fmt, ...);
]]

function prova(itemNum)
	--ITEM_INFO* Items;
	--item = ffi.C.Items[itemNum];
	--ffi.C.SoundEffect(56, item.pos, 0); 
	
	ffi.C.printf("Test\n");
end
