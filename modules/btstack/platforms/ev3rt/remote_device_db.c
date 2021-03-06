/*
 * Copyright (C) 2014 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at 
 * contact@bluekitchen-gmbh.com
 *
 */

#include <string.h>
#include <stdlib.h>

#include "remote_device_db.h"
#include "btstack_memory.h"
#include "debug.h"
#include "btstack-interface.h"

#include <btstack/utils.h>
#include <btstack/linked_list.h>

// This lists should be only accessed by tests.
linked_list_t db_mem_link_keys = NULL;
linked_list_t db_mem_names = NULL;
static linked_list_t db_mem_services = NULL;

// Device info
static void db_open(void){
}

static void db_close(void){ 
}

static db_mem_device_t * get_item(linked_list_t list, bd_addr_t bd_addr) {
    linked_item_t *it;
    for (it = (linked_item_t *) list; it ; it = it->next){
        db_mem_device_t * item = (db_mem_device_t *) it;
        if (BD_ADDR_CMP(item->bd_addr, bd_addr) == 0) {
            return item;
        }
    }
    return NULL;
}

static int get_name(bd_addr_t bd_addr, device_name_t *device_name) {
    db_mem_device_name_t * item = (db_mem_device_name_t *) get_item(db_mem_names, bd_addr);
    
    if (!item) return 0;
    
    strncpy((char*)device_name, item->device_name, MAX_NAME_LEN);
    
	linked_list_remove(&db_mem_names, (linked_item_t *) item);
    linked_list_add(&db_mem_names, (linked_item_t *) item);
	
	return 1;
}

typedef unsigned char bool;
#define true  (1)
#define false (0)
#define BD_ADDR_STRLEN (BD_ADDR_LEN * 2)
#define LINKKEY_STRLEN (3 /* store link_key_type */ + LINK_KEY_LEN * 2)

static void memory_to_dbstr(char *dst, const uint8_t *src, int size) {
	for (int i = 0; i < size; ++i) {
		int val[2] = {src[i] / 16, src[i] % 16};
		for (int j = 0; j < 2; ++j) {
			if (val[j] < 10)
				*dst++ = '0' + val[j];
			else
				*dst++ = 'a' + val[j] - 10;
		}
	}
	*dst = '\0';
}

static void dbstr_to_memory(uint8_t *dst, const char *src) {
	while (*src != '\0') {
		int val = 0;
		for (int j = 0; j < 2; ++j) {
			if ('0' <= src[j] && src[j] <= '9')
				val += (src[j] - '0') * (1 + (1 - j) * 15);
			else
				val += (src[j] - 'a' + 10) * (1 + (1 - j) * 15);
		}
		*dst++ = val;
		src += 2;
	}
}


static void btstack_db_append_linkkey(db_mem_device_link_key_t *item) {
    char bd_addr_str[BD_ADDR_STRLEN + 1];
    char linkkey_str[LINKKEY_STRLEN + 1];
    memory_to_dbstr(bd_addr_str, item->device.bd_addr, BD_ADDR_LEN);
	memory_to_dbstr(linkkey_str, &(item->link_key_type), 1);
	memory_to_dbstr(linkkey_str + 3, item->link_key, LINK_KEY_LEN);
	linkkey_str[2] = '/';
    btstack_db_append(bd_addr_str, linkkey_str);
}

void btstack_db_cache_flush() {
    linked_item_t *it;

    btstack_db_lock();
    btstack_db_append(NULL, NULL);
    for (it = (linked_item_t *) db_mem_link_keys; it ; it = it->next){
        btstack_db_append_linkkey((db_mem_device_link_key_t *)it);
    }
    btstack_db_unlock();
}

static int get_link_key(bd_addr_t bd_addr, link_key_t link_key, link_key_type_t * link_key_type) {
    db_mem_device_link_key_t * item = (db_mem_device_link_key_t *) get_item(db_mem_link_keys, bd_addr);
    
    if (!item) return 0;
    
    memcpy(link_key, item->link_key, LINK_KEY_LEN);
    if (link_key_type) {
        *link_key_type = item->link_key_type;
    }
	linked_list_remove(&db_mem_link_keys, (linked_item_t *) item);
    linked_list_add(&db_mem_link_keys, (linked_item_t *) item);

	return 1;
}

static int get_link_key_and_sync(bd_addr_t bd_addr, link_key_t link_key, link_key_type_t * link_key_type) {
    int ret;
    btstack_db_lock();
    ret = get_link_key(bd_addr, link_key, link_key_type);
    btstack_db_unlock();
    return ret;
}

static void delete_link_key(bd_addr_t bd_addr){
    db_mem_device_t * item = get_item(db_mem_link_keys, bd_addr);
    
    if (!item) return;

    linked_list_remove(&db_mem_link_keys, (linked_item_t *) item);
    btstack_memory_db_mem_device_link_key_free((db_mem_device_link_key_t*)item);
}

static void delete_link_key_and_sync(bd_addr_t bd_addr){
    btstack_db_lock();
    delete_link_key(bd_addr);
    btstack_db_unlock();
    btstack_db_cache_flush();
}

static void put_link_key(bd_addr_t bd_addr, link_key_t link_key, link_key_type_t link_key_type){

    // check for existing record and remove if found
    db_mem_device_link_key_t * record = (db_mem_device_link_key_t *) get_item(db_mem_link_keys, bd_addr);
    if (record){
        linked_list_remove(&db_mem_link_keys, (linked_item_t*) record);
    }

    // record not found, get new one from memory pool
    if (!record) {
        record = btstack_memory_db_mem_device_link_key_get();
    }

    // if none left, re-use last item and remove from list
    if (!record){
        record = (db_mem_device_link_key_t*)linked_list_get_last_item(&db_mem_link_keys);
        if (record) {
            linked_list_remove(&db_mem_link_keys, (linked_item_t*) record);
        }
    }
        
    if (!record) {
        log_error("%s() failed.", __FUNCTION__);
        return;
    }
    
    memcpy(record->device.bd_addr, bd_addr, sizeof(bd_addr_t));
    memcpy(record->link_key, link_key, LINK_KEY_LEN);
    record->link_key_type = link_key_type;
    linked_list_add(&db_mem_link_keys, (linked_item_t *) record);
}

static void put_link_key_and_sync(bd_addr_t bd_addr, link_key_t link_key, link_key_type_t link_key_type){
    btstack_db_lock();
    put_link_key(bd_addr, link_key, link_key_type);
    btstack_db_unlock();
    btstack_db_cache_flush();
}

void btstack_db_cache_put(const char *key, const char *value) {
    if (strlen(key) == BD_ADDR_STRLEN && strlen(value) == LINKKEY_STRLEN) {
        bd_addr_t bd_addr;
        link_key_t link_key;
        link_key_type_t link_key_type;

        char link_key_type_str[3] = { value[0], value[1], '\0' };
        dbstr_to_memory(bd_addr, key);
        dbstr_to_memory(link_key, value + 3);
        dbstr_to_memory(&link_key_type, link_key_type_str);

        btstack_db_lock();
        put_link_key(bd_addr, link_key, link_key_type);
        btstack_db_unlock();
#if defined(DEBUG_BTSTACK)
        log_error("%s(): addr 0x%08x%04x, link_key_type %d", __FUNCTION__, (bd_addr[0] << 24L) | (bd_addr[1] << 16L) | (bd_addr[2] << 8L) | bd_addr[3], (bd_addr[4] << 8L) | bd_addr[5], link_key_type);
        log_error("    link_key 0x%08x%08x%08x%08x", (link_key[0] << 24L) | (link_key[1] << 16L) | (link_key[2] << 8L) | link_key[3], (link_key[4] << 24L) | (link_key[5] << 16L) | (link_key[6] << 8L) | link_key[7],
                (link_key[8] << 24L) | (link_key[9] << 16L) | (link_key[10] << 8L) | link_key[11], (link_key[12] << 24L) | (link_key[13] << 16L) | (link_key[14] << 8L) | link_key[15]);
#endif
    } else log_error("%s() failed.", __FUNCTION__);
}

static void delete_name(bd_addr_t bd_addr){
    db_mem_device_t * item = get_item(db_mem_names, bd_addr);
    
    if (!item) return;
    
    linked_list_remove(&db_mem_names, (linked_item_t *) item);
    btstack_memory_db_mem_device_name_free((db_mem_device_name_t*)item);    
}

static void put_name(bd_addr_t bd_addr, device_name_t *device_name){

    // check for existing record and remove if found
    db_mem_device_name_t * record = (db_mem_device_name_t *) get_item(db_mem_names, bd_addr);
    if (record){
        linked_list_remove(&db_mem_names, (linked_item_t*) record);
    }

    // record not found, get new one from memory pool
    if (!record) {
        record = btstack_memory_db_mem_device_name_get();
    }

    // if none left, re-use last item and remove from list
    if (!record){
        record = (db_mem_device_name_t*)linked_list_get_last_item(&db_mem_names);
        if (record) {
            linked_list_remove(&db_mem_names, (linked_item_t*) record);
        }
    }

    if (!record) return;
    
    memcpy(record->device.bd_addr, bd_addr, sizeof(bd_addr_t));
    strncpy(record->device_name, (const char*) device_name, MAX_NAME_LEN);
    linked_list_add(&db_mem_names, (linked_item_t *) record);
}


// MARK: PERSISTENT RFCOMM CHANNEL ALLOCATION

static uint8_t persistent_rfcomm_channel(char *serviceName){
    linked_item_t *it;
    db_mem_service_t * item;
    uint8_t max_channel = 1;

    for (it = (linked_item_t *) db_mem_services; it ; it = it->next){
        item = (db_mem_service_t *) it;
        if (strncmp(item->service_name, serviceName, MAX_NAME_LEN) == 0) {
            // Match found
            return item->channel;
        }

        // TODO prevent overflow
        if (item->channel >= max_channel) max_channel = item->channel + 1;
    }

    // Allocate new persistant channel
    db_mem_service_t * newItem = btstack_memory_db_mem_service_get();

    if (!newItem) return 0;
    
    strncpy(newItem->service_name, serviceName, MAX_NAME_LEN);
    newItem->channel = max_channel;
    linked_list_add(&db_mem_services, (linked_item_t *) newItem);
    return max_channel;
}


const remote_device_db_t remote_device_db_memory = {
    db_open,
    db_close,
    get_link_key_and_sync,
    put_link_key_and_sync,
    delete_link_key_and_sync,
    get_name,
    put_name,
    delete_name,
    persistent_rfcomm_channel
};
