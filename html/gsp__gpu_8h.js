var gsp__gpu_8h =
[
    [ "InterruptRelayQueue", "namespace_service_1_1_g_s_p.html#struct_service_1_1_g_s_p_1_1_interrupt_relay_queue", [
      [ "error_code", "namespace_service_1_1_g_s_p.html#a419d25f966883049df02e70f783735e7", null ],
      [ "index", "namespace_service_1_1_g_s_p.html#abbbbae950e3993b128afbafa290e246e", null ],
      [ "missed_PDC0", "namespace_service_1_1_g_s_p.html#a26550eb999ac4f5be146d14e28f3e9ad", null ],
      [ "missed_PDC1", "namespace_service_1_1_g_s_p.html#a1e96a11c469c2874b5d050521e43f761", null ],
      [ "number_interrupts", "namespace_service_1_1_g_s_p.html#a41a61e0a99a77158d8deaa00c95ac998", null ],
      [ "padding1", "namespace_service_1_1_g_s_p.html#a69eacc54ebb6240d2b511f79b362329e", null ],
      [ "slot", "namespace_service_1_1_g_s_p.html#a0989927ff7f90352d72e459410dfa055", null ]
    ] ],
    [ "FrameBufferInfo", "namespace_service_1_1_g_s_p.html#struct_service_1_1_g_s_p_1_1_frame_buffer_info", [
      [ "active_fb", "namespace_service_1_1_g_s_p.html#ac115d4e9749310f5eadcb496815d6ba9", null ],
      [ "address_left", "namespace_service_1_1_g_s_p.html#a51828331447c75fd40a8f8e7fee21b90", null ],
      [ "address_right", "namespace_service_1_1_g_s_p.html#adac621ec40c6197fa7358ea8f7960c1b", null ],
      [ "format", "namespace_service_1_1_g_s_p.html#a59e442b7db91a33d067a082cbb12e331", null ],
      [ "shown_fb", "namespace_service_1_1_g_s_p.html#a45dbdcfe26980711749fc08c823fc65e", null ],
      [ "stride", "namespace_service_1_1_g_s_p.html#af8d7aece89a70e2b964458ecc43dc318", null ],
      [ "unknown", "namespace_service_1_1_g_s_p.html#aa67510d63fe5a6cdb8e5046b4fea63f9", null ]
    ] ],
    [ "FrameBufferUpdate", "namespace_service_1_1_g_s_p.html#struct_service_1_1_g_s_p_1_1_frame_buffer_update", [
      [ "framebuffer_info", "namespace_service_1_1_g_s_p.html#a7a954c20c858d0a8dbfedf1110c9dfdb", null ],
      [ "index", "namespace_service_1_1_g_s_p.html#a9e693156a5c17d17f695804db668425c", null ],
      [ "is_dirty", "namespace_service_1_1_g_s_p.html#ae76d512be25d243d8fed21ef5ac4fa9a", null ],
      [ "pad1", "namespace_service_1_1_g_s_p.html#a53a11adbd13ca2bbbc4acc060c792731", null ],
      [ "pad2", "namespace_service_1_1_g_s_p.html#a029a124644de5269356305177d5bc157", null ]
    ] ],
    [ "Command", "namespace_service_1_1_g_s_p.html#struct_service_1_1_g_s_p_1_1_command", [
      [ "id", "namespace_service_1_1_g_s_p.html#a535656106559def5361bf46dd59795d8", null ]
    ] ],
    [ "CommandBuffer", "namespace_service_1_1_g_s_p.html#struct_service_1_1_g_s_p_1_1_command_buffer", [
      [ "commands", "namespace_service_1_1_g_s_p.html#a59ab7cbaf20e2bd5691710687a0f15b7", null ],
      [ "unk", "namespace_service_1_1_g_s_p.html#a3942d056e257bf3263a5122b1efe589d", null ]
    ] ],
    [ "SessionData", "struct_service_1_1_g_s_p_1_1_session_data.html", "struct_service_1_1_g_s_p_1_1_session_data" ],
    [ "GSP_GPU", "class_service_1_1_g_s_p_1_1_g_s_p___g_p_u.html", "class_service_1_1_g_s_p_1_1_g_s_p___g_p_u" ],
    [ "Command.__unnamed__", "namespace_service_1_1_g_s_p.html#union_service_1_1_g_s_p_1_1_command_8____unnamed____", [
      [ "cache_flush", "namespace_service_1_1_g_s_p.html#a3394e031c97d36c774b2bf79a781b7c1", null ],
      [ "display_transfer", "namespace_service_1_1_g_s_p.html#af551f02cab233b894093e41a0c7f7746", null ],
      [ "dma_request", "namespace_service_1_1_g_s_p.html#ad4bd8cbd62f5afe24092088398c02239", null ],
      [ "memory_fill", "namespace_service_1_1_g_s_p.html#a86d1a66dc832c47ba1b1910f0716d923", null ],
      [ "raw_data", "namespace_service_1_1_g_s_p.html#a2b8f5d5bf88f34992e835c30aa1d586e", null ],
      [ "submit_gpu_cmdlist", "namespace_service_1_1_g_s_p.html#adefc85d168e111f241d5dc0e4da8ff27", null ],
      [ "texture_copy", "namespace_service_1_1_g_s_p.html#ae40028927c343909fbf2674108e07fa7", null ]
    ] ],
    [ "Command.__unnamed__.dma_request", "namespace_service_1_1_g_s_p.html#struct_service_1_1_g_s_p_1_1_command_8____unnamed_____8dma__request", [
      [ "dest_address", "namespace_service_1_1_g_s_p.html#a565349b861225d01dd6ef4f50d3b81d4", null ],
      [ "size", "namespace_service_1_1_g_s_p.html#af7bd60b75b29d79b660a2859395c1a24", null ],
      [ "source_address", "namespace_service_1_1_g_s_p.html#a3304064f8831d7b81c197364accbb7ee", null ]
    ] ],
    [ "Command.__unnamed__.submit_gpu_cmdlist", "namespace_service_1_1_g_s_p.html#struct_service_1_1_g_s_p_1_1_command_8____unnamed_____8submit__gpu__cmdlist", [
      [ "address", "namespace_service_1_1_g_s_p.html#a884d9804999fc47a3c2694e49ad2536a", null ],
      [ "do_flush", "namespace_service_1_1_g_s_p.html#ae7ede7ac6c35d12c8a64926e7604070f", null ],
      [ "flags", "namespace_service_1_1_g_s_p.html#a4e5868d676cb634aa75b125a0f741abf", null ],
      [ "size", "namespace_service_1_1_g_s_p.html#af7bd60b75b29d79b660a2859395c1a24", null ],
      [ "unused", "namespace_service_1_1_g_s_p.html#acee8dfb8ec13d13feb7a9f4ba988c865", null ]
    ] ],
    [ "Command.__unnamed__.memory_fill", "namespace_service_1_1_g_s_p.html#struct_service_1_1_g_s_p_1_1_command_8____unnamed_____8memory__fill", [
      [ "control1", "namespace_service_1_1_g_s_p.html#a9140d6cde2c74313da35842720cde88b", null ],
      [ "control2", "namespace_service_1_1_g_s_p.html#a20f4c8fe7ba41be84ffd4030df4f2a6d", null ],
      [ "end1", "namespace_service_1_1_g_s_p.html#a83203a1c7d69b2d1cd39a58d25c52a5e", null ],
      [ "end2", "namespace_service_1_1_g_s_p.html#aa227eb244663fd30c07e2b79ca007859", null ],
      [ "start1", "namespace_service_1_1_g_s_p.html#a368096b6864b055eebc12fb33ae636ae", null ],
      [ "start2", "namespace_service_1_1_g_s_p.html#a7692d94d0f2624777aae2ead3e5a8213", null ],
      [ "value1", "namespace_service_1_1_g_s_p.html#a9946687e5fa0dab5993ededddb398d2e", null ],
      [ "value2", "namespace_service_1_1_g_s_p.html#af066ce9385512ee02afc6e14d627e9f2", null ]
    ] ],
    [ "Command.__unnamed__.display_transfer", "namespace_service_1_1_g_s_p.html#struct_service_1_1_g_s_p_1_1_command_8____unnamed_____8display__transfer", [
      [ "flags", "namespace_service_1_1_g_s_p.html#a4e5868d676cb634aa75b125a0f741abf", null ],
      [ "in_buffer_address", "namespace_service_1_1_g_s_p.html#adae03a308e6ad38eba775408ad3929ea", null ],
      [ "in_buffer_size", "namespace_service_1_1_g_s_p.html#aa8a61b6b083eecec95e1dd01d543ec47", null ],
      [ "out_buffer_address", "namespace_service_1_1_g_s_p.html#aa7f25ae1f12c35ca73561ef9c09795b9", null ],
      [ "out_buffer_size", "namespace_service_1_1_g_s_p.html#a7a103970396bedb191f4fe99b807de35", null ]
    ] ],
    [ "Command.__unnamed__.texture_copy", "namespace_service_1_1_g_s_p.html#struct_service_1_1_g_s_p_1_1_command_8____unnamed_____8texture__copy", [
      [ "flags", "namespace_service_1_1_g_s_p.html#a4e5868d676cb634aa75b125a0f741abf", null ],
      [ "in_buffer_address", "namespace_service_1_1_g_s_p.html#adae03a308e6ad38eba775408ad3929ea", null ],
      [ "in_width_gap", "namespace_service_1_1_g_s_p.html#ab3fc116d873ea12b60cb5ca9fb579f8f", null ],
      [ "out_buffer_address", "namespace_service_1_1_g_s_p.html#aa7f25ae1f12c35ca73561ef9c09795b9", null ],
      [ "out_width_gap", "namespace_service_1_1_g_s_p.html#a3e949b85f4933a9d3a06b64f118dc91c", null ],
      [ "size", "namespace_service_1_1_g_s_p.html#af7bd60b75b29d79b660a2859395c1a24", null ]
    ] ],
    [ "Command.__unnamed__.cache_flush", "namespace_service_1_1_g_s_p.html#struct_service_1_1_g_s_p_1_1_command_8____unnamed_____8cache__flush", [
      [ "regions", "namespace_service_1_1_g_s_p.html#a5d8f0e36aae02a7560a86e92a2c28a79", null ]
    ] ],
    [ "Command.__unnamed__.cache_flush.regions", "namespace_service_1_1_g_s_p.html#struct_service_1_1_g_s_p_1_1_command_8____unnamed_____8cache__flush_8regions", [
      [ "address", "namespace_service_1_1_g_s_p.html#a884d9804999fc47a3c2694e49ad2536a", null ],
      [ "size", "namespace_service_1_1_g_s_p.html#af7bd60b75b29d79b660a2859395c1a24", null ]
    ] ],
    [ "CommandBuffer.__unnamed__", "namespace_service_1_1_g_s_p.html#union_service_1_1_g_s_p_1_1_command_buffer_8____unnamed____", [
      [ "hex", "namespace_service_1_1_g_s_p.html#ab8d1b43eae73587ba56baef574709ecb", null ],
      [ "index", "namespace_service_1_1_g_s_p.html#a6a992d5529f459a44fee58c733255e86", null ],
      [ "number_commands", "namespace_service_1_1_g_s_p.html#a6705a32d6943b537d8b0bf1223f4552f", null ]
    ] ],
    [ "CommandId", "gsp__gpu_8h.html#a72ee9e58a96151ccb6b037f53fe1e5c6", [
      [ "REQUEST_DMA", "gsp__gpu_8h.html#a72ee9e58a96151ccb6b037f53fe1e5c6a7e0bc94c6f3e0adb5763b5fc6e4420f4", null ],
      [ "SUBMIT_GPU_CMDLIST", "gsp__gpu_8h.html#a72ee9e58a96151ccb6b037f53fe1e5c6a7a24f4eea44140d57370b79ab2e686ee", null ],
      [ "SET_MEMORY_FILL", "gsp__gpu_8h.html#a72ee9e58a96151ccb6b037f53fe1e5c6a65aa52b2daedc87f4d37d28eff59f71b", null ],
      [ "SET_DISPLAY_TRANSFER", "gsp__gpu_8h.html#a72ee9e58a96151ccb6b037f53fe1e5c6a1bb3d7b297126796b0710fdc5cb66af7", null ],
      [ "SET_TEXTURE_COPY", "gsp__gpu_8h.html#a72ee9e58a96151ccb6b037f53fe1e5c6a93063378e0f39626c01db3e352a9bec3", null ],
      [ "CACHE_FLUSH", "gsp__gpu_8h.html#a72ee9e58a96151ccb6b037f53fe1e5c6aefdad4194c14be14b62100b14abfc85a", null ]
    ] ],
    [ "InterruptId", "gsp__gpu_8h.html#a835de04a145f2548358c65e13c24c704", [
      [ "PSC0", "gsp__gpu_8h.html#a835de04a145f2548358c65e13c24c704a8e6c0b58d2f09e75a22ecc9e7a23a8ed", null ],
      [ "PSC1", "gsp__gpu_8h.html#a835de04a145f2548358c65e13c24c704a9abd5029630f63c8355c27107e4002b7", null ],
      [ "PDC0", "gsp__gpu_8h.html#a835de04a145f2548358c65e13c24c704ae6c0e228eb8745e7fdcbf07e86b06cdd", null ],
      [ "PDC1", "gsp__gpu_8h.html#a835de04a145f2548358c65e13c24c704a910402e28a9bc36f9f05f776276d2e3f", null ],
      [ "PPF", "gsp__gpu_8h.html#a835de04a145f2548358c65e13c24c704aabe13dc25ca714087237993f33111647", null ],
      [ "P3D", "gsp__gpu_8h.html#a835de04a145f2548358c65e13c24c704a8cbd30bd7192c58f1dcec3f0024475ac", null ],
      [ "DMA", "gsp__gpu_8h.html#a835de04a145f2548358c65e13c24c704a33fd5f6391f2f0cb4c91179d7f521949", null ]
    ] ],
    [ "SetBufferSwap", "gsp__gpu_8h.html#a6491853dac89e80bafb42ba130fb9239", null ]
];