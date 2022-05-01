
// WARNING: Variable defined which should be unmapped: var_28h
// WARNING: Could not reconcile some variable overlaps

ulong main(ulong argc, int64_t argv)

{
    int64_t iVar1;
    int32_t iVar2;
    ulong uVar3;
    uint64_t uVar4;
    int64_t in_FS_OFFSET;
    int64_t iStack144;
    int64_t s;
    ulong var_74h;
    int32_t var_64h;
    int32_t var_60h;
    int64_t var_5ch;
    char *var_50h;
    int64_t var_48h;
    ulong *var_40h;
    int64_t canary;
    ulong var_28h;
    
    canary = *(in_FS_OFFSET + 0x28);
    s = argv;
    var_74h._0_4_ = argc;
    if (argc == 2) {
        iStack144 = 0x11fb;
        iVar2 = sym.imp.strlen(*(argv + 8));
        if (iVar2 == 6) {
            iStack144 = 0x1213;
            iVar2 = sym.checker(*(s + 8));
            if (iVar2 != 0) {
                iStack144 = 0x1240;
                var_5ch._0_4_ = sym.imp.strlen(*(s + 8));
                var_50h = *(s + 8);
                var_48h = var_5ch + -1;
                uVar4 = (var_5ch * 8 + 0xfU) / 0x10;
                iVar1 = uVar4 * -0x10;
                var_40h = &main::s + uVar4 * -2;
                *var_40h = 1;
                for (var_60h = 1; var_60h < var_5ch; var_60h = var_60h + 1) {
                    var_40h[var_60h] = (var_40h[var_60h + -1] * 0x83) % 0x3b9aca09;
                }
                stack0xffffffffffffffa0 = *var_50h;
                for (var_64h = 1; var_64h < var_5ch; var_64h = var_64h + 1) {
                    stack0xffffffffffffffa0 =
                         stack0xffffffffffffffa0 + ((var_50h[var_64h] % 0x3b9aca09) * var_40h[var_64h]) % 0x3b9aca09;
                }
                if (stack0xffffffffffffffa0 == 0x3c0431a5) {
                    *(&stack0xffffffffffffff70 + iVar1) = 0x1396;
                    sym.imp.puts("Key is valid");
                }
                else {
                    *(&stack0xffffffffffffff70 + iVar1) = 0x13a4;
                    sym.imp.puts("Wrong key!");
                }
                uVar3 = 0;
                goto code_r0x000013a9;
            }
        }
    }
    iStack144 = 0x1223;
    sym.imp.puts(0x2010);
    uVar3 = 1;
code_r0x000013a9:
    if (canary != *(in_FS_OFFSET + 0x28)) {
        iStack144 = 0x13c0;
        uVar3 = sym.imp.__stack_chk_fail();
    }
    return uVar3;
}
