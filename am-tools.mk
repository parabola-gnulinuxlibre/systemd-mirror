CCLD = $(CC)
AM_CPPFLAGS += -MT $@ -MD -MP -MF $(@D)/$(DEPDIR)/$(basename $(@F)).P$(patsubst .%,%,$(suffix $(@F)))

COMPILE   = $(CC) $(ALL_CPPFLAGS) $(ALL_CFLAGS)
LTCOMPILE = $(LIBTOOL) $(AM_V_lt) --tag=CC $(ALL_LIBTOOLFLAGS) --mode=compile $(CC) $(ALL_CPPFLAGS) $(ALL_CFLAGS)
LINK      = $(LIBTOOL) $(AM_V_lt) --tag=CC $(ALL_LIBTOOLFLAGS) --mode=link $(CCLD) $(ALL_CFLAGS) $(ALL_LDFLAGS) -o $@
