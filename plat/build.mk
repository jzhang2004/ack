
define build-platform-headers
$(call reset)
$(eval q := $D/include/$(strip $1))
$(call installto, $(PLATIND)/$(PLATFORM)/include/$(strip $1))
$(eval PLATFORM_HEADERS_$(PLATFORM) += $q)
endef

define build-platform-impl
    $(call reset)
	$(eval q := $D/descr)
	$(call installto, $(PLATIND)/descr/$(PLATFORM))

	$(eval PLATFORM_$(PLATFORM) := \
			$(PLATIND)/descr/$(PLATFORM) \
			$(PLATFORM_HEADERS_$(PLATFORM)) \
			$(PLATDEP)/$(PLATFORM)/as \
			$(PLATDEP)/$(PLATFORM)/ncg \
			$(ARCHITECTURE_$(ARCH)))

	$(foreach f, $(platform-headers), $(call build-platform-headers, $f))

	# libsys

	$(call reset)
	$(foreach f, $(platform-libsys), $(call ackfile, $D/libsys/$f))
	$(call acklibrary, $(LIBDIR)/$(PLATFORM)/libsys.a)
	$(call installto, $(PLATIND)/$(PLATFORM)/libsys.a)

    # libem

    $(call reset)
    $(eval objdir := $(PLATFORM))
    $(eval ackflags += -Ih)
    $(foreach f, $(arch-libem-$(ARCH)), $(call ackfile, mach/$(ARCH)/libem/$f))
    $(call acklibrary, $(LIBDIR)/$(PLATFORM)/libem.a)
    $(call installto, $(PLATIND)/$(PLATFORM)/libem.a)

    # libend

    $(call reset)
    $(eval objdir := $(PLATFORM))
    $(eval ackflags += -Ih)
    $(foreach f, $(arch-libend-$(ARCH)), $(call ackfile, mach/$(ARCH)/libend/$f))
    $(call acklibrary, $(LIBDIR)/$(PLATFORM)/libend.a)
    $(call installto, $(PLATIND)/$(PLATFORM)/libend.a)

    # The tools themselves

    $(call build-as)
    $(call build-ncg)

	# Build top only if the architecture asks for it.

	$(if $(arch-top-$(ARCH)), $(call build-top))

    # Language runtimes

    $(foreach runtime, $(RUNTIMES), $(build-runtime-$(runtime)))
endef

build-platform = $(eval $(call build-platform-impl, $1))


