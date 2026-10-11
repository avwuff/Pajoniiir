# Keep the scheduling metadata in fixed internal RAM for the M3 HS bench.
# Audio payload and other endpoint lists retain the pinned PSRAM policy.
set(_isoc_pool_anchor [=[
static dma_buffer_block_t *buffer_block_alloc(usb_transfer_type_t type)
]=])
set(_isoc_pool_helpers [=[
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC && CONFIG_USB_HOST_DWC_DMA_CAP_MEMORY_IN_PSRAM
#define PAJONIIIR_INTERNAL_ISOC_LISTS 4u
/* Two HCD buffers each for one audio OUT and one feedback IN pipe. Static
 * internal BSS avoids 512-aligned heap allocation after tracks fragment RAM.
 * No live list can be reused: HCD calls free only after pipe retirement. */
static uint32_t s_internal_isoc_lists[PAJONIIIR_INTERNAL_ISOC_LISTS][128]
    __attribute__((aligned(512)));
static uint32_t s_internal_isoc_claimed;
_Static_assert(XFER_LIST_LEN_ISOC * sizeof(usb_dwc_ll_dma_qtd_t) == 512,
               "Review internal ISO pool for this pinned descriptor layout");
#endif
static void *pajoniiir_descriptor_alloc(usb_transfer_type_t type, size_t bytes)
{
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC && CONFIG_USB_HOST_DWC_DMA_CAP_MEMORY_IN_PSRAM
    if (type == USB_TRANSFER_TYPE_ISOCHRONOUS) {
        if (bytes != 512u) return NULL;
        for (unsigned i = 0; i < PAJONIIIR_INTERNAL_ISOC_LISTS; ++i) {
            const uint32_t bit = 1u << i;
            uint32_t old = __atomic_load_n(&s_internal_isoc_claimed, __ATOMIC_RELAXED);
            while (!(old & bit)) {
                if (__atomic_compare_exchange_n(&s_internal_isoc_claimed, &old,
                        old | bit, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)) {
                    memset(s_internal_isoc_lists[i], 0, 512u);
                    return s_internal_isoc_lists[i];
                }
            }
        }
        return NULL; /* Never silently fall back to PSRAM or internal heap. */
    }
#else
    (void)type;
#endif
    return heap_caps_aligned_calloc(USB_DWC_QTD_LIST_MEM_ALIGN, bytes, 1, XFER_DESC_LIST_CAPS);
}
static void pajoniiir_descriptor_free(void *list)
{
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC && CONFIG_USB_HOST_DWC_DMA_CAP_MEMORY_IN_PSRAM
    for (unsigned i = 0; i < PAJONIIIR_INTERNAL_ISOC_LISTS; ++i) {
        if (list == s_internal_isoc_lists[i]) {
            const uint32_t bit = 1u << i;
            const uint32_t old = __atomic_fetch_and(&s_internal_isoc_claimed,
                                                  ~bit, __ATOMIC_RELEASE);
            assert(old & bit);
            (void)old;
            return;
        }
    }
#endif
    heap_caps_free(list);
}
static uint32_t pajoniiir_descriptor_caps(usb_transfer_type_t type)
{
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC && CONFIG_USB_HOST_DWC_DMA_CAP_MEMORY_IN_PSRAM
    if (type == USB_TRANSFER_TYPE_ISOCHRONOUS)
        return MALLOC_CAP_DMA | MALLOC_CAP_CACHE_ALIGNED | MALLOC_CAP_INTERNAL;
#else
    (void)type;
#endif
    return XFER_DESC_LIST_CAPS;
}

static dma_buffer_block_t *buffer_block_alloc(usb_transfer_type_t type)
]=])
set(_isoc_alloc_original "heap_caps_aligned_calloc(USB_DWC_QTD_LIST_MEM_ALIGN, desc_list_len * sizeof(usb_dwc_ll_dma_qtd_t), 1, XFER_DESC_LIST_CAPS)")
set(_isoc_alloc_patched "pajoniiir_descriptor_alloc(type, desc_list_len * sizeof(usb_dwc_ll_dma_qtd_t))")
set(_isoc_error_original "heap_caps_free(xfer_desc_list);")
set(_isoc_error_patched "pajoniiir_descriptor_free(xfer_desc_list);")
set(_isoc_free_original "heap_caps_free(buffer->xfer_desc_list);")
set(_isoc_free_patched "pajoniiir_descriptor_free(buffer->xfer_desc_list);")
set(_isoc_align_original "esp_cache_get_alignment(XFER_DESC_LIST_CAPS, &cache_align);")
set(_isoc_align_patched "esp_cache_get_alignment(pajoniiir_descriptor_caps(type), &cache_align);")
foreach(_isoc_change pool alloc error free align)
    if(_isoc_change STREQUAL "pool")
        set(_isoc_old "${_isoc_pool_anchor}")
        set(_isoc_new "${_isoc_pool_helpers}")
    else()
        set(_isoc_old "${_isoc_${_isoc_change}_original}")
        set(_isoc_new "${_isoc_${_isoc_change}_patched}")
    endif()
    string(FIND "${_pajoniiir_hcd_contents}" "${_isoc_old}" _isoc_at)
    if(_isoc_at EQUAL -1)
        message(FATAL_ERROR "Pinned HCD ISO descriptor ${_isoc_change} contract changed")
    endif()
    string(REPLACE "${_isoc_old}" "${_isoc_new}" _pajoniiir_hcd_contents "${_pajoniiir_hcd_contents}")
endforeach()
