#include "ticket.h"
#include "support.h"
#include "unittype.h"
#include "aes.h"
#include "sha.h"
#include "rsa.h"
#include "ff.h"

u32 ValidateTicket(Ticket* ticket) {
    const u8 magic[] = { TICKET_SIG_TYPE };
    if ((memcmp(ticket->sig_type, magic, sizeof(magic)) != 0) ||
        ((strncmp((char*) ticket->issuer, TICKET_ISSUER, 0x40) != 0) &&
        (strncmp((char*) ticket->issuer, TICKET_ISSUER_DEV, 0x40) != 0)) ||
        (ticket->commonkey_idx >= 6))
        return 1;
    return 0;
}

u32 ValidateTicketSignature(Ticket* ticket) {
    static bool got_modexp = false;
    static u8 mod[0x100] = { 0 };
    static u32 exp = 0;
    
    // grab cert from cert.db
    if (!got_modexp) {
        Certificate cert;
        FIL db;
        UINT bytes_read;
        if (f_open(&db, "1:/dbs/certs.db", FA_READ | FA_OPEN_EXISTING) != FR_OK)
            return 1;
        f_lseek(&db, 0x3F10);
        f_read(&db, &cert, CERT_SIZE, &bytes_read);
        f_close(&db);
        memcpy(mod, cert.mod, 0x100);
        exp = getle32(cert.exp);
        got_modexp = true;
    }
    
    if (!RSA_setKey2048(3, mod, exp) ||
        !RSA_verify2048((void*) &(ticket->signature), (void*) &(ticket->issuer), 0x210))
        return 1;
        
    return 0;
}

u32 CryptTitleKey(TitleKeyEntry* tik, bool encrypt, bool devkit) {
    // From https://github.com/profi200/Project_CTR/blob/master/makerom/pki/prod.h#L19
    static const u8 common_keyy[6][16] __attribute__((aligned(16))) = {
        {0xD0, 0x7B, 0x33, 0x7F, 0x9C, 0xA4, 0x38, 0x59, 0x32, 0xA2, 0xE2, 0x57, 0x23, 0x23, 0x2E, 0xB9} , // 0 - eShop Titles
        {0x0C, 0x76, 0x72, 0x30, 0xF0, 0x99, 0x8F, 0x1C, 0x46, 0x82, 0x82, 0x02, 0xFA, 0xAC, 0xBE, 0x4C} , // 1 - System Titles
        {0xC4, 0x75, 0xCB, 0x3A, 0xB8, 0xC7, 0x88, 0xBB, 0x57, 0x5E, 0x12, 0xA1, 0x09, 0x07, 0xB8, 0xA4} , // 2
        {0xE4, 0x86, 0xEE, 0xE3, 0xD0, 0xC0, 0x9C, 0x90, 0x2F, 0x66, 0x86, 0xD4, 0xC0, 0x6F, 0x64, 0x9F} , // 3
        {0xED, 0x31, 0xBA, 0x9C, 0x04, 0xB0, 0x67, 0x50, 0x6C, 0x44, 0x97, 0xA3, 0x5B, 0x78, 0x04, 0xFC} , // 4
        {0x5E, 0x66, 0x99, 0x8A, 0xB4, 0xE8, 0x93, 0x16, 0x06, 0x85, 0x0F, 0xD7, 0xA1, 0x6D, 0xD7, 0x55} , // 5
    };
    // From https://github.com/profi200/Project_CTR/blob/master/makerom/pki/dev.h#L21
    static const u8 common_key_dev[6][16] __attribute__((aligned(16))) = {
        {0x55, 0xA3, 0xF8, 0x72, 0xBD, 0xC8, 0x0C, 0x55, 0x5A, 0x65, 0x43, 0x81, 0x13, 0x9E, 0x15, 0x3B} , // 0 - eShop Titles
        {0x44, 0x34, 0xED, 0x14, 0x82, 0x0C, 0xA1, 0xEB, 0xAB, 0x82, 0xC1, 0x6E, 0x7B, 0xEF, 0x0C, 0x25} , // 1 - System Titles
        {0xF6, 0x2E, 0x3F, 0x95, 0x8E, 0x28, 0xA2, 0x1F, 0x28, 0x9E, 0xEC, 0x71, 0xA8, 0x66, 0x29, 0xDC} , // 2
        {0x2B, 0x49, 0xCB, 0x6F, 0x99, 0x98, 0xD9, 0xAD, 0x94, 0xF2, 0xED, 0xE7, 0xB5, 0xDA, 0x3E, 0x27} , // 3
        {0x75, 0x05, 0x52, 0xBF, 0xAA, 0x1C, 0x04, 0x07, 0x55, 0xC8, 0xD5, 0x9A, 0x55, 0xF9, 0xAD, 0x1F} , // 4
        {0xAA, 0xDA, 0x4C, 0xA8, 0xF6, 0xE5, 0xA9, 0x77, 0xE0, 0xA0, 0xF9, 0xE4, 0x76, 0xCF, 0x0D, 0x63} , // 5
    };
    
    u32 mode = (encrypt) ? AES_CNT_TITLEKEY_ENCRYPT_MODE : AES_CNT_TITLEKEY_DECRYPT_MODE;
    u8 ctr[16] = { 0 };
    
    // setup key 0x3D // ctr
    if (tik->commonkey_idx >= 6) return 1;
    if (!devkit) setup_aeskeyY(0x3D, (void*) common_keyy[tik->commonkey_idx]);
    else setup_aeskey(0x3D, (void*) common_key_dev[tik->commonkey_idx]);
    use_aeskey(0x3D);
    memcpy(ctr, tik->title_id, 8);
    set_ctr(ctr);
    
    // decrypt / encrypt the titlekey
    aes_decrypt(tik->titlekey, tik->titlekey, 1, mode);
    return 0;
}

u32 GetTitleKey(u8* titlekey, Ticket* ticket) {
    TitleKeyEntry tik = { 0 };
    memcpy(tik.title_id, ticket->title_id, 8);
    memcpy(tik.titlekey, ticket->titlekey, 16);
    tik.commonkey_idx = ticket->commonkey_idx;
    
    if (CryptTitleKey(&tik, false, TICKET_DEVKIT(ticket)) != 0) return 1;
    memcpy(titlekey, tik.titlekey, 16);
    return 0;
}

Ticket* TicketFromTickDbChunk(u8* chunk, u8* title_id, bool legit_pls) {
    // chunk must be aligned to 0x200 byte in file and at least 0x400 byte big
    Ticket* tick = (Ticket*) (chunk + 0x18);
    if ((getle32(chunk + 0x10) == 0) || (getle32(chunk + 0x14) != sizeof(Ticket))) return NULL;
    if (ValidateTicket(tick) != 0) return NULL; // ticket not validated
    if (title_id && (memcmp(title_id, tick->title_id, 8) != 0)) return NULL; // title id not matching
    if (legit_pls && (ValidateTicketSignature(tick) != 0)) return NULL; // legit check using RSA sig
    
    return tick;
}

u32 FindTicket(Ticket* ticket, u8* title_id, bool force_legit, bool emunand) {
    const char* path_db = TICKDB_PATH(emunand); // EmuNAND / SysNAND
    const u32 area_offsets[] = { TICKDB_AREA_OFFSETS };
    u8 data[0x400];
    FIL file;
    UINT btr;
    
    // parse file, sector by sector
    if (f_open(&file, path_db, FA_READ | FA_OPEN_EXISTING) != FR_OK)
        return 1;
    bool found = false;
    for (u32 p = 0; p < 2; p++) {
        u32 area_offset = area_offsets[p];
        for (u32 i = 0; !found && (i < TICKDB_AREA_SIZE); i += 0x200) {
            f_lseek(&file, area_offset + i);
            if ((f_read(&file, data, 0x400, &btr) != FR_OK) || (btr != 0x400)) break;
            Ticket* tick = TicketFromTickDbChunk(data, title_id, force_legit);
            if (!tick) continue;
            memcpy(ticket, tick, sizeof(Ticket));
            found = true;
            break;
        }
    }
    f_close(&file);
    
    return (found) ? 0 : 1;
}

u32 FindTitleKey(Ticket* ticket, u8* title_id) {
    bool found = false;
    // search for a titlekey inside encTitleKeys.bin / decTitleKeys.bin
    // when found, add it to the ticket
    for (u32 enc = 0; (enc <= 1) && !found; enc++) {
        TitleKeysInfo* tikdb = (TitleKeysInfo*) (TEMP_BUFFER + (TEMP_BUFFER_SIZE/2));
        u32 len = LoadSupportFile((enc) ? TIKDB_NAME_ENC : TIKDB_NAME_DEC, tikdb, (TEMP_BUFFER_SIZE/2));
        
        if (len == 0) continue; // file not found
        if (tikdb->n_entries > (len - 16) / 32)
            continue; // filesize / titlekey db size mismatch
        for (u32 t = 0; t < tikdb->n_entries; t++) {
            TitleKeyEntry* tik = tikdb->entries + t;
            if (memcmp(title_id, tik->title_id, 8) != 0)
                continue;
            if (!enc && (CryptTitleKey(tik, true, TICKET_DEVKIT(ticket)) != 0)) // encrypt the key first
                continue;
            memcpy(ticket->titlekey, tik->titlekey, 16);
            ticket->commonkey_idx = tik->commonkey_idx;
            found = true; // found, inserted
            break;
        }
    }
    
    return (found) ? 0 : 1;
}

u32 AddTitleKeyToInfo(TitleKeysInfo* tik_info, TitleKeyEntry* tik_entry, bool decrypted_in, bool decrypted_out, bool devkit) {
    if (!tik_entry) { // no titlekey entry -> reset database
        memset(tik_info, 0, 16);
        return 0;
    }
    // check if entry already in DB
    u32 n_entries = tik_info->n_entries;
    TitleKeyEntry* tik = tik_info->entries;
    for (u32 i = 0; i < n_entries; i++, tik++)
        if (memcmp(tik->title_id, tik_entry->title_id, 8) == 0) return 0;
    // actually a new titlekey
    memcpy(tik, tik_entry, sizeof(TitleKeyEntry));
    if ((decrypted_in != decrypted_out) && (CryptTitleKey(tik, !decrypted_out, devkit) != 0)) return 1;
    tik_info->n_entries++;
    return 0;
}

u32 AddTicketToInfo(TitleKeysInfo* tik_info, Ticket* ticket, bool decrypt) { // TODO: check for legit tickets?
    if (!ticket) return AddTitleKeyToInfo(tik_info, NULL, false, false, false);
    TitleKeyEntry tik = { 0 };
    memcpy(tik.title_id, ticket->title_id, 8);
    memcpy(tik.titlekey, ticket->titlekey, 16);
    tik.commonkey_idx = ticket->commonkey_idx;
    return AddTitleKeyToInfo(tik_info, &tik, false, decrypt, TICKET_DEVKIT(ticket));
}

u32 BuildFakeTicket(Ticket* ticket, u8* title_id) {
    const u8 sig_type[4] =  { TICKET_SIG_TYPE }; // RSA_2048 SHA256
    const u8 ticket_cnt_index[] = { // whatever this is
        0x00, 0x01, 0x00, 0x14, 0x00, 0x00, 0x00, 0xAC, 0x00, 0x00, 0x00, 0x14, 0x00, 0x01, 0x00, 0x14,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x84,
        0x00, 0x00, 0x00, 0x84, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
    };
    // set ticket all zero for a clean start
    memset(ticket, 0x00, sizeof(Ticket));
    // fill ticket values
    memcpy(ticket->sig_type, sig_type, 4);
    memset(ticket->signature, 0xFF, 0x100);
    snprintf((char*) ticket->issuer, 0x40, IS_DEVKIT ? TICKET_ISSUER_DEV : TICKET_ISSUER);
    memset(ticket->ecdsa, 0xFF, 0x3C);
    ticket->version = 0x01;
    memset(ticket->titlekey, 0xFF, 16);
    memcpy(ticket->title_id, title_id, 8);
    ticket->commonkey_idx = 0x00; // eshop
    ticket->audit = 0x01; // whatever
    memcpy(ticket->content_index, ticket_cnt_index, sizeof(ticket_cnt_index));
    
    return 0;
}

u32 BuildTicketCert(u8* tickcert) {
    const u8 cert_hash_expected[0x20] = {
        0xDC, 0x15, 0x3C, 0x2B, 0x8A, 0x0A, 0xC8, 0x74, 0xA9, 0xDC, 0x78, 0x61, 0x0E, 0x6A, 0x8F, 0xE3, 
        0xE6, 0xB1, 0x34, 0xD5, 0x52, 0x88, 0x73, 0xC9, 0x61, 0xFB, 0xC7, 0x95, 0xCB, 0x47, 0xE6, 0x97
    };
    const u8 cert_hash_expected_dev[0x20] = {
        0x97, 0x2A, 0x32, 0xFF, 0x9D, 0x4B, 0xAA, 0x2F, 0x1A, 0x24, 0xCF, 0x21, 0x13, 0x87, 0xF5, 0x38,
        0xC6, 0x4B, 0xD4, 0x8F, 0xDF, 0x13, 0x21, 0x3D, 0xFC, 0x72, 0xFC, 0x8D, 0x9F, 0xDD, 0x01, 0x0E
    };
    
    // open certs.db file on SysNAND
    FIL db;
    UINT bytes_read;
    if (f_open(&db, "1:/dbs/certs.db", FA_READ | FA_OPEN_EXISTING) != FR_OK)
        return 1;
    // grab ticket cert from 3 offsets
    f_lseek(&db, 0x3F10);
    f_read(&db, tickcert + 0x000, 0x300, &bytes_read);
    f_lseek(&db, 0x0C10);
    f_read(&db, tickcert + 0x300, 0x1F0, &bytes_read);
    f_lseek(&db, 0x3A00);
    f_read(&db, tickcert + 0x4F0, 0x210, &bytes_read);
    f_close(&db);
    
    // check the certificate hash
    u8 cert_hash[0x20];
    sha_quick(cert_hash, tickcert, TICKET_CDNCERT_SIZE, SHA256_MODE);
    if (memcmp(cert_hash, IS_DEVKIT ? cert_hash_expected_dev : cert_hash_expected, 0x20) != 0)
        return 1;
    
    return 0;
}
