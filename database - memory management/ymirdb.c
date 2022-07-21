#define FREE(ptr) do{ \
    free((ptr));      \
    (ptr) = NULL;     \
  }while(0)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <ctype.h> 
#include <limits.h>
#include "ymirdb.h"

entry* get_entry(entry* head, char* line) { 
	char* key = strtok(line, " ");
	if (key[strlen(key) - 1] == '\n') 
		key[strlen(key) - 1] = 0; 

	while(head != NULL) { 
		if (!(strcmp(head->key, key))) { 
			return head;
		}
		head = head->next; 
	}
	return NULL; 
}

void update_type(entry* ent) { 
	for (int i = 0; i < ent->length; i++) { 
		if (ent->values[i].type == ENTRY) { 
			ent->is_simple = 0; 
			return; 
		}
	}
	ent->is_simple = 1; 
}

int cmp( const void* a, const void* b ) {
  return strcmp( *(const char**)a, *(const char**)b );
}

void push(entry* head, entry* ent, char* line) { 
	int len = 1; 
	int k = 0; 
	char* rem = line; 
	char line2[MAX_LINE]; 
	strcpy(line2, line); 
	char* token = strtok_r(rem, " ", &rem); 
	element new_element;
	entry* entry_pushed; 

	for(int i = 0; i < strlen(rem); i++) { 
		if (rem[i] == ' ')
			len++; 
	}

	char* rem2 = line2; 
	char* check = strtok_r(rem2, " ", &rem2); 
	check = strtok_r(rem2, " ", &rem2);
	while(check) { 
		if (isalpha(check[0])) { 
			entry_pushed = get_entry(head, check); 
			if (entry_pushed == NULL) { 
				printf("no such key\n\n"); 
				return; 
			}
			if (!strcmp(entry_pushed->key, ent->key)) { 
				printf("not permitted\n\n"); 
				return; 
			}
		}
		check = strtok_r(rem2, " ", &rem2);
	}
 
	token = strtok_r(rem, " ", &rem); 
	if (token == NULL) { 
		printf("empty push\n\n");
		return; 
	}
	//Copy array into temp:
	element* temp = malloc(sizeof(element) * ent->length); 
	for(int i = 0; i < ent->length; i++)  
		temp[i] = ent->values[i]; 

	ent->length += len;
	ent->values = realloc(ent->values, ent->length * sizeof(entry)); 

	while(token) { 
		if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) { 
			new_element.type = INTEGER; 
			new_element.value = atoi(token); 
		} else if (isalpha(token[0])) {
			entry_pushed = get_entry(head, token);

			// Entry to push to: 
			ent->is_simple = 0; 
			ent->forward[ent->forward_size] = entry_pushed;
			ent->forward_size++;
			ent->forward_max++; 
			ent->forward = realloc(ent->forward, ent->forward_max * sizeof(entry*)); 

			// Entry pushed: 
			new_element.type = ENTRY; 
			new_element.entry = entry_pushed;
			new_element.entry->backward[new_element.entry->backward_size] = ent; 
			new_element.entry->backward_size++;  
			new_element.entry->backward_max++; 
			new_element.entry->backward = realloc(new_element.entry->backward, 
				new_element.entry->backward_max * sizeof(entry*));
		}
		ent->values[len - 1 - k++] = new_element; 
		token = strtok_r(rem, " ", &rem);  
	}

	// Copy elements back into array: 
	for(int i = len; i < ent->length; i++) { 
		ent->values[i] = temp[i - len]; 
	}

	FREE(temp); 
	printf("ok\n\n"); 
}

void append(entry* head, entry* ent, char* line) { 
	int len = 1; 
	int prev_len = ent->length; 
	char* rem = line; 
	char line2[MAX_LINE]; 
	strcpy(line2, line); 
	char* token = strtok_r(rem, " ", &rem);
	int k = 0; 
	element new_element;
	entry* entry_appended; 

	for(int i = 0; i < strlen(rem); i++) { 
		if (rem[i] == ' ')
			len++; 
	}

	char* rem2 = line2; 
	char* check = strtok_r(rem2, " ", &rem2); 
	check = strtok_r(rem2, " ", &rem2);
	while(check) { 
		if (isalpha(check[0])) { 
			entry_appended = get_entry(head, check); 
			if (entry_appended == NULL) { 
				printf("no such key\n\n"); 
				return; 
			}
			if (!strcmp(entry_appended->key, ent->key)) { 
				printf("not permitted\n\n"); 
				return; 
			}
		}
		check = strtok_r(rem2, " ", &rem2);
	}
	token = strtok_r(rem, " ", &rem); 
	if (token == NULL) { 
		printf("empty append\n\n"); 
		return; 
	}

	ent->length += len;
	ent->values = realloc(ent->values, ent->length * sizeof(entry)); 

	while(token) { 
		if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) { 
			new_element.type = INTEGER; 
			new_element.value = atoi(token); 
		} else if (isalpha(token[0])) {
			entry_appended = get_entry(head, token); 

			// Entry to push to: 
			ent->is_simple = 0; 
			ent->forward[ent->forward_size] = entry_appended;
			ent->forward_size++;
			ent->forward_max++; 
			ent->forward = realloc(ent->forward, ent->forward_max * sizeof(entry*)); 

			// Entry pushed: 
			new_element.type = ENTRY; 
			new_element.entry = entry_appended;
			new_element.entry->backward[new_element.entry->backward_size] = ent; 
			new_element.entry->backward_size++;  
			new_element.entry->backward_max++; 
			new_element.entry->backward = realloc(new_element.entry->backward, 
				new_element.entry->backward_max * sizeof(entry*));
		}
		ent->values[prev_len + k++] = new_element;  
		token = strtok_r(rem, " ", &rem); 
	}
	printf("ok\n\n"); 
}

entry* create_entry(char* line, entry* head, int create_references) { 
	int len = 1;
	int k = 0; 
	char* rem = line; 
	char line2[MAX_LINE]; 
	strcpy(line2, line); 
	char* rem2 = line2; 
	char* token = strtok_r(rem, " ", &rem); 
	element new_element;
	entry* new_entry = malloc(sizeof(entry));
	entry* entry_in_list; 
	
	// Count how many values there are:
	for(int i = 0; i < strlen(rem); i++) { 
		if (rem[i] == ' ')
			len++; 
	} 
	strtok_r(rem2, " ", &rem2); 
	if (strtok_r(rem2, " ", &rem2) == NULL) len = 0; 

	// Check if length is greater than 15 characters 
	if (strlen(token) > 15) { 
		printf("Key is longer than 15 characters.\n\n");
		new_entry->is_successful = 0; 
		return new_entry; 
	} 

	if (!isalpha(token[0])) { 
		printf("Key must start with a letter.\n\n"); 
		new_entry->is_successful = 0; 
		return new_entry; 
	}

	// Set the entry's key: 
	memset(new_entry->key, 0, MAX_KEY); 
	for(int i = 0; i < strlen(token); i++)
		if (token[i] != '\n')
			new_entry->key[i] = token[i];

	// Allocate memory for array: 
	new_entry->length = len;
	new_entry->values = calloc(new_entry->length, sizeof(element));	
 
	new_entry->backward_size = 0; 
	new_entry->forward_size = 0; 
 
	new_entry->backward_max = 1;
	new_entry->forward_max = 1;

	new_entry->backward = calloc(new_entry->backward_max, sizeof(entry*)); 
	new_entry->forward = calloc(new_entry->forward_max, sizeof(entry*)); 
	
	new_entry->is_simple = 1;
	new_entry->is_snapshot = 0; 
	// Add elements to the entry's values.
	token = strtok_r(rem, " ", &rem); 
	while (token) {   
		if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) { 
			new_element.type = INTEGER; 
			new_element.value = atoi(token); 
		} else if (isalpha(token[0]) && create_references) {
			entry_in_list = get_entry(head, token); 
			if (entry_in_list == NULL) { 
				printf("no such key\n\n"); 
				new_entry->is_successful = 0; 
				return new_entry; 
			}

			if (!strcmp(entry_in_list->key, new_entry->key)) { 
				printf("not permitted\n\n"); 
				new_entry->is_successful = 0; 
				return new_entry; 
			}

			// New Entry (Entered): 
			new_entry->is_simple = 0; 
			new_entry->forward[new_entry->forward_size] = entry_in_list;
			new_entry->forward_size++;
			new_entry->forward_max++; 
			new_entry->forward = realloc(new_entry->forward, 
				new_entry->forward_max * sizeof(entry*)); 

			// Entry in list: 
			new_element.type = ENTRY; 
			new_element.entry = entry_in_list;  
			new_element.entry->backward[new_element.entry->backward_size] = new_entry; 
			new_element.entry->backward_size++;  
			new_element.entry->backward_max++; 
			new_element.entry->backward = realloc(new_element.entry->backward, 
				new_element.entry->backward_max * sizeof(entry*));
		}
 		new_entry->values[k++] = new_element; 
		token = strtok_r(rem, " ", &rem); 
	}	

	new_entry->is_successful = 1; 
	return new_entry; 
}

void get_backward_references(entry* ent, char*** keys, int* len, int* size) {
	for(int i = 0; i < ent->backward_size; i++) { 
		if (ent->backward[i]->backward_size != 0)
			get_backward_references(ent->backward[i], keys, len, size);   
	} 

	*size = ent->backward_size; 
	*len += ent->backward_size;
	(*keys) = realloc((*keys), sizeof(char*) * ((*len) + ent->backward_size)); 
	for (int i = 0; i < ent->backward_size; i++) { 
		(*keys)[(*len) - *size + i] = ent->backward[i]->key; 
	}
}

void print_backward(entry* head, char* line) { 
	char* rem = line; 
	char* token = strtok_r(rem, " ", &rem); 
	entry* ent = get_entry(head, token);  
	if (ent == NULL) { 
		printf("no such key\n\n");
		return; 
	} 
	if (ent->backward_size == 0) { 
		printf("nil\n\n");
		return;
	}
	int len = 0; 
	int size = 0; 
	char** keys = malloc(sizeof(char)); 
	get_backward_references(ent, &keys, &len, &size); 

	// Only keep unique keys: 
	for (int i = 0; i < len; i++) { 
		for (int j = i + 1; j < len; j++) { 
			if (!strcmp(keys[i], keys[j])) { 
				keys[i] = " "; 
			}
		}
	}

	qsort(keys, len, sizeof(keys[0]), cmp);

	for(int i = 0; i < len; i++) { 
		if (strcmp(keys[i], " ")) { 
			printf("%s", keys[i]); 
			if (i != len - 1)
				printf(", ");
		}
	}
	printf("\n\n");
	FREE(keys); 
}

void get_forward_references(entry* ent, char*** keys, int* len, int* size) {
	for(int i = 0; i < ent->forward_size; i++) { 
		if (ent->forward[i]->forward_size != 0)
			get_forward_references(ent->forward[i], keys, len, size);   
	} 

	*size = ent->forward_size; 
	*len += ent->forward_size;
	(*keys) = realloc((*keys), sizeof(char*) * ((*len) + ent->forward_size)); 
	for (int i = 0; i < ent->forward_size; i++) { 
		(*keys)[(*len) - *size + i] = ent->forward[i]->key; 
	}
}

void print_forward(entry* head, char* line) { 
	char* rem = line; 
	char* token = strtok_r(rem, " ", &rem); 
	entry* ent = get_entry(head, token);  
	if (ent == NULL) { 
		printf("no such key\n\n");
		return; 
	} 
	if (ent->forward_size == 0) { 
		printf("nil\n\n");
		return;
	}
	int len = 0; 
	int size = 0; 
	char** keys = malloc(sizeof(char)); 
	get_forward_references(ent, &keys, &len, &size); 

	// Only keep unique keys: 
	for (int i = 0; i < len; i++) { 
		for (int j = i + 1; j < len; j++) { 
			if (!strcmp(keys[i], keys[j])) { 
				keys[i] = " "; 
			}
		}
	}

	qsort(keys, len, sizeof(keys[0]), cmp);

	for(int i = 0; i < len; i++) {  
		if (strcmp(keys[i], " ")) { 
			printf("%s", keys[i]); 
			if (i != len - 1)
				printf(", ");
		}
	}
	printf("\n\n");
	FREE(keys); 
}

void type(entry* head, char* line) { 
	entry* ent = get_entry(head, line); 
	if (ent == NULL) { 
		printf("no such key\n\n");
		return; 
	}
	if (ent->is_simple) { 
		printf("simple\n\n"); 
	} else { 
		printf("general\n\n"); 
	}
}

void print_entry(entry* ent) { 
	printf("["); 
	for(int i = 0; i < ent->length; i++) {
		if (ent->values[i].type == INTEGER) { 
			printf("%d", ent->values[i].value); 
		} else if (ent->values[i].type == ENTRY) { 
			printf("%s", ent->values[i].entry->key);
		}
		if (i != ent->length - 1)
			printf(" ");
	}
	printf("]\n"); 
	return; 
}

void apply_snapshot(entry** head, entry* ent, int with_reference) { 
	// Get values in a string:
	char line[MAX_LINE]; 
	strcpy(line, ent->key); 
	strcat(line, " "); 
	for (int i = 0; i < ent->length; i++) { 
		if (ent->values[i].type == INTEGER) { 
			char value[MAX_INPUT]; 
			sprintf(value, "%d", ent->values[i].value); 
			if (i != ent->length - 1)
				strcat(value, " "); 
			strcat(line, value);
		}
		else { 
			strcat(line, ent->values[i].entry->key); 
			if (i != ent->length - 1)
				strcat(line, " "); 
		}
	}

	// Make a copy of the values in the snapshot without references first: 
	if (ent->is_snapshot && !with_reference) { 
		entry* new_entry = create_entry(line, *head, 0); 
		ent = new_entry; 

		entry* last = *head; 
		ent->next = NULL; 
		if (*head == NULL) { 
			ent->prev = NULL; 
			*head = ent; 
			return; 
		}

		while (last->next != NULL) 
			last = last->next; 
		
		last->next = ent; 
		ent->prev = last; 
		return;
	}

	// Fill in values with references: 
	if (with_reference) { 
		entry* ptr = *head; 
		while (strcmp(ptr->key, ent->key))
			ptr = ptr->next; 
		element elem; 
		int k = 0; 
		entry* entry_in_list; 
		char* rem = line; 
		char* token = strtok_r(rem, " ", &rem);  
		token = strtok_r(rem, " ", &rem); 

		while (token) {   
			if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) { 
				elem.type = INTEGER; 
				elem.value = atoi(token); 
			} else if (isalpha(token[0])) {
				entry_in_list = get_entry(*head, token); 

				// New Entry (Entered): 
				ptr->is_simple = 0; 
				ptr->forward[ptr->forward_size] = entry_in_list;
				ptr->forward_size++;
				ptr->forward_max++; 
				ptr->forward = realloc(ptr->forward, ptr->forward_max * sizeof(entry*)); 

				// Entry in list: 
				elem.type = ENTRY; 
				elem.entry = entry_in_list;  
				elem.entry->backward[elem.entry->backward_size] = ptr; 
				elem.entry->backward_size++;  
				elem.entry->backward_max++; 
				elem.entry->backward = realloc(elem.entry->backward, 
					elem.entry->backward_max * sizeof(entry*));
			}
			ptr->values[k++] = elem; 
			token = strtok_r(rem, " ", &rem); 
		}
	}	
}

void append_to_entries(entry** head, entry* ent) { 
	entry* last = *head; 
		ent->next = NULL; 
		if (*head == NULL) { 
			ent->prev = NULL; 
			*head = ent; 
			return; 
		}

		while (last->next != NULL) 
			last = last->next; 
		
		last->next = ent; 
		ent->prev = last; 
		return;
}

int delete(entry** head, char* line, snapshot* snap) { 
	entry* ent = get_entry(*head, line);
	entry** entries; 
	int k = 0; 

	if (ent == NULL) { 
		printf("no such key\n\n"); 
		return 0; 
	}
	
	if (ent->backward_size != 0) { 
		printf("not permitted\n\n"); 
		return 0; 
	}

	// For each entry in ent: 
	for(int i = 0; i < ent->forward_size; i++) {
		//Create array to store all backward  
		entries = calloc(ent->forward[i]->backward_size, sizeof(entry*)); 
		for(int j = 0; j < ent->forward[i]->backward_size; j++) 
			entries[j] = ent->forward[i]->backward[j]; 

		ent->forward[i]->backward_size--; 
		ent->forward[i]->backward_max--; 
		FREE(ent->forward[i]->backward); 
		ent->forward[i]->backward = calloc(ent->forward[i]->backward_max, sizeof(entry*)); 

		for(int j = 0; j < ent->forward[i]->backward_size + 1; j++) { 
			if (entries[j] != ent) 
				ent->forward[i]->backward[k++] = entries[j]; 
		}
		k = 0; 
		FREE(entries); 
	}

	if (*head == ent)
		*head = ent->next;
	if (ent->next != NULL) 
		ent->next->prev = ent->prev;
	if (ent->prev != NULL) 
		ent->prev->next = ent->next; 
    
	FREE(ent->values); 
	FREE(ent->backward); 
	FREE(ent->forward);
	if (!ent->is_snapshot) 
		FREE(ent); 
	return 1; 
}

void update_entry(entry* head, entry* ent, char* line) {
	int k = 0; 
	int len = 1;
	char* rem = line; 
	char line2[MAX_LINE]; 
	strcpy(line2, line); 
	char* token = strtok_r(rem, " ", &rem); 
	element new_element; 
	entry* entry_in_list; 

	for(int i = 0; i < strlen(rem); i++) { 
		if (rem[i] == ' ')
			len++; 
	} 

	char* rem2 = line2; 
	char* check = strtok_r(rem2, " ", &rem2); 
	check = strtok_r(rem2, " ", &rem2);
	if (check == NULL) 
		len = 0; 
	while(check) { 
		if (isalpha(check[0])) { 
			entry_in_list = get_entry(head, check); 
			if (entry_in_list == NULL) { 
				printf("no such key\n\n"); 
				return; 
			}
			if (!strcmp(entry_in_list->key, ent->key)) { 
				printf("not permitted\n\n"); 
				return; 
			}
		}
		check = strtok_r(rem2, " ", &rem2);
	}

	// Remove backward references to the updated entry  
	for(int i = 0; i < ent->length; i++) { 
		if (ent->values[i].type == ENTRY) { 
			entry* tmp = ent->values[i].entry; 
			entry** copy = calloc(tmp->backward_size, sizeof(entry*));
			for (int j = 0; j < tmp->backward_size; j++)
				copy[j] = tmp->backward[j]; 
			
			tmp->backward_size--;
			tmp->backward_max--; 
			tmp->backward = realloc(tmp->backward, tmp->backward_max * sizeof(entry*)); 

			for (int j = 0; j < tmp->backward_size; j++)
				if (copy[j]->key != ent->key)
					tmp->backward[k++] = copy[j]; 
			FREE(copy);
		}
	} k = 0; 
	
	// Set default values for entry: 
	ent->length = len; 
	FREE(ent->values); 
	ent->values = calloc(ent->length, sizeof(entry));

	ent->forward_size = 0; 
	ent->forward_max = 1; 
	FREE(ent->forward); 
	ent->forward = calloc(ent->forward_max, sizeof(entry*)); 

	ent->is_simple = 1; 
	ent->is_snapshot = 0; 

	token = strtok_r(rem, " ", &rem); 
	while(token) { 
		if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) { 
			new_element.type = INTEGER; 
			new_element.value = atoi(token); 
		} else if (isalpha(token[0])) {
			entry_in_list = get_entry(head, token); 

			// Entry to update: 
			ent->is_simple = 0; 
			ent->forward[ent->forward_size] = entry_in_list;
			ent->forward_size += 1;
			ent->forward_max += ent->forward_size; 
			ent->forward = realloc(ent->forward, ent->forward_max * sizeof(entry*)); 

			// Entry in list: 
			new_element.type = ENTRY; 
			new_element.entry = entry_in_list;  
			new_element.entry->backward[new_element.entry->backward_size] = ent; 
			new_element.entry->backward_size++;  
			new_element.entry->backward_max++; 
			new_element.entry->backward = realloc(new_element.entry->backward, 
				new_element.entry->backward_max * sizeof(entry*));
		}
		ent->values[k++] = new_element;
		token = strtok_r(rem, " ", &rem); 
	}
	printf("ok\n\n"); 
}

void pluck(entry* head, char* line) { 
	char* rem = line; 
	char* token = strtok_r(rem, " ", &rem); 
	int k = 0;  
	int do_once = 1;
	char* val = strtok_r(rem, " ", &rem); 
	if (val == NULL) { 
		printf("no index specified\n\n"); 
		return; 
	}
	int index = atoi(val); 
	entry* ent = get_entry(head, token);
	if (ent == NULL) { 
		printf("no such key\n\n"); 
		return; 
	}
	if (index > ent->length || index <= 0 ) { 
		printf("index out of range\n\n"); 
		return; 
	} 
	element value = ent->values[index - 1];
	element* elements = calloc(ent->length, sizeof(element));
	entry** entries;

	if (value.type == ENTRY)  
		printf("%s\n\n", value.entry->key);
	else 
		printf("%d\n\n", value.value);

	// Copy array 
	for(int i = 0; i < ent->length; i++) 
		elements[i] = ent->values[i]; 

	ent->length--; 
	ent->values = realloc(ent->values, sizeof(element) * ent->length); 

	if (value.type == ENTRY) {
		entries = calloc(value.entry->backward_size, sizeof(entry*)); 
		// Make a copy of all the backward references: 
		for(int i = 0; i < value.entry->backward_size; i++)
			entries[i] = value.entry->backward[i];
		
		// Reduce backward size: 
		value.entry->backward_max--; 
		value.entry->backward_size--; 

		// Reallocate memory to be 1 smaller: 
		value.entry->backward = realloc(value.entry->backward, 
			sizeof(entry*) * value.entry->backward_max); 

		// Copy all of the elements except the backward reference 
		for(int i = 0; i < value.entry->backward_size + 1; i++) {
			if (entries[i] == ent && do_once) { 
				do_once = 0; 
				continue; 
			} else { 
				value.entry->backward[k++] = entries[i]; 
			} 	 
		}
		
		k = 0; 
		do_once = 1;
		entries = realloc(entries, sizeof(entry*) * ent->forward_size); 
		// Make a copy of all the forward references: 
		for(int i = 0; i < ent->forward_size; i++)
			entries[i] = ent->forward[i];
		
		// Reduce forward size: 
		ent->forward_max--; 
		ent->forward_size--; 

		// Reallocate forward reference array to be 1 smaller: 
		ent->forward = realloc(ent->forward, sizeof(entry*) * ent->forward_max); 

		// Copy all of the elements except the forward reference: 
		for(int i = 0; i < ent->forward_size + 1; i++) { 
			if (entries[i] == value.entry && do_once) { 
				do_once = 0; 
				continue;
			} else { 
				ent->forward[k++] = entries[i];
			} 
		}
		FREE(entries);
	}

	k = 0; 
	for(int i = 0; i < ent->length + 1; i++) { 
		if (value.type == ENTRY) { 
			if (elements[i].entry != value.entry)
				ent->values[k++] = elements[i]; 
		} else { 
			if(elements[i].value != value.value) 
				ent->values[k++] = elements[i];
		}
	}
	
	FREE(elements); 
	update_type(ent); 
	return; 
}

void pop(entry* head, char* line) { 
	char* rem = line; 
	char* token = strtok_r(rem, " ", &rem); 
	int k = 0; 
	int do_once = 1;
	entry* ent = get_entry(head, token); 
	if (ent == NULL) { 
		printf("no such key\n\n");
		return; 
	}

	if (ent->length == 0) { 
		printf("nil\n\n"); 
		return; 
	}
	element value = ent->values[0];
	element* elements = calloc(ent->length, sizeof(element));
	entry** entries;


	if (value.type == ENTRY)  
		printf("%s\n\n", value.entry->key);
	else 
		printf("%d\n\n", value.value);

	// Copy array 
	for(int i = 0; i < ent->length; i++) 
		elements[i] = ent->values[i]; 

	ent->length--; 
	ent->values = realloc(ent->values, sizeof(element) * ent->length); 
 
	for(int i = 0; i < ent->length + 1; i++) { 
		if (value.type == ENTRY) { 
			if (elements[i].entry != value.entry)
				ent->values[k++] = elements[i]; 
		} else { 
			if(elements[i].value != value.value) 
				ent->values[k++] = elements[i];
		}
	} k = 0;

	if (value.type == ENTRY) {
		entries = calloc(value.entry->backward_size, sizeof(entry*)); 
		// Make a copy of all the backward references: 
		for(int i = 0; i < value.entry->backward_size; i++)
			entries[i] = value.entry->backward[i];
		
		// Reduce backward size: 
		value.entry->backward_max--; 
		value.entry->backward_size--; 

		// Reallocate memory to be 1 smaller: 
		value.entry->backward = realloc(value.entry->backward, 
			sizeof(entry*) * value.entry->backward_max); 

		// Copy all of the elements except the backward reference 
		for(int i = 0; i < value.entry->backward_size + 1; i++) 
			if (entries[i] == ent && do_once) { 
				do_once = 0; 
				continue; 
			} else { 
				value.entry->backward[k++] = entries[i];
			}
		
		k = 0; 
		do_once = 1;
		entries = realloc(entries, sizeof(entry*) * ent->forward_size); 
		// Make a copy of all the forward references: 
		for(int i = 0; i < ent->forward_size; i++)
			entries[i] = ent->forward[i];
		
		// Reduce forward size: 
		ent->forward_max--; 
		ent->forward_size--; 

		// Reallocate forward reference array to be 1 smaller: 
		ent->forward = realloc(ent->forward, sizeof(entry*) * ent->forward_max); 

		// Copy all of the elements except the forward reference: 
		for(int i = 0; i < ent->forward_size + 1; i++) 
			if (entries[i] == value.entry && do_once) { 
				do_once = 0; 
				continue; 
			} else { 
				ent->forward[k++] = entries[i]; 
			}
		FREE(entries);
	}
	
	FREE(elements); 
	update_type(ent); 
	return; 
}

void pick(entry* head, char* line) { 
	char* rem = line; 
	char* token = strtok_r(rem, " ", &rem); 
	entry* ent = get_entry(head, token);
	if (ent == NULL) { 
		printf("no such key\n\n"); 
		return; 
	}
	if (ent->length == 0) { 
		printf("nil\n\n"); 
		return; 
	}

	int index = atoi(strtok_r(rem, " ", &rem)); 
	if (index > ent->length || index <= 0 ) { 
		printf("index out of range\n\n"); 
		return; 
	}
	element value = ent->values[index - 1];

	if (value.type == ENTRY) 
		printf("%s\n\n", value.entry->key);
	else 
		printf("%d\n\n", value.value);

}

void purge(entry** head, snapshot* snap_head, char* line) {
	char* rem = line; 
	char* token = strtok_r(rem, " ", &rem); 
	int all_valid = 1;  
	int found = 1; 
	entry* ent = get_entry(*head, token); 
	snapshot* ptr = snap_head; 

	// Check if the entry can be deleted from the current state: 
	if (ent != NULL && ent->backward_size != 0)
		all_valid = 0; 
	// Check if the entry can be deleted from every state: 
	else { 
		while(ptr != NULL) { 
			for(int i = 0; i < ptr->len; i++)
				if (!strcmp(ptr->entries[i].key, token)) { 
					found = 1;
					if (ptr->entries[i].backward_size != 0) { 
						all_valid = 0;
					}
				}
			ptr = ptr->next; 
		}
	} ptr = snap_head; 

	// Delete the entry from all states: 
	if (all_valid) {
		if (ent != NULL)
			delete(head, line, NULL); 
		while (ptr != NULL)  {
			for(int i = 0; i < ptr->len; i++) { 
				if (!strcmp(ptr->entries[i].key, token)) { 
					delete(&ptr->head, line, ptr); 
				}
			}
			ptr = ptr->next; 
		}
		printf("ok\n\n"); 
		return;
	} else { 
		printf("not permitted\n\n"); 
		return; 
	}

	if (!found) { 
		printf("no such key\n\n"); 
		return; 
	} 
}

snapshot* create_snapshot(entry* head) { 
	snapshot* new_snapshot = malloc(sizeof(snapshot)); 
	entry* ptr = head; 
	if (head == NULL) { 
		new_snapshot->len = 0;
		return new_snapshot;
	} 
	int len = 1;  

	while(ptr->next != NULL) { 
		ptr = ptr->next; 
		len++;
	} ptr = head; 

	new_snapshot->len = len;
	new_snapshot->entries = calloc(len, sizeof(entry));
	new_snapshot->head = new_snapshot->entries;

	// Create copies of all the entries without references in order to create a scaffold to 
	// later fill in references:
	for(int i = 0; i < len; i++) { 
		strcpy(new_snapshot->entries[i].key, ptr->key);

		new_snapshot->entries[i].backward_max = ptr->backward_max; 
		new_snapshot->entries[i].forward_max = ptr->forward_max; 
		new_snapshot->entries[i].backward_size = ptr->backward_size; 
		new_snapshot->entries[i].forward_size = ptr->forward_size; 
		new_snapshot->entries[i].length = ptr->length; 
		new_snapshot->entries[i].is_simple = ptr->is_simple; 
		new_snapshot->entries[i].is_successful = ptr->is_successful;
		new_snapshot->entries[i].is_snapshot = 1; 

		ptr = ptr->next; 
	} ptr = head; 

	// Fill in values that require a reference (Backward/Forward, Next/Prev, and Values):
	for(int i = 0; i < len; i++) { 
		new_snapshot->entries[i].values = calloc(ptr->length, sizeof(element)); 
		new_snapshot->entries[i].backward = calloc(ptr->backward_max, sizeof(entry*)); 
		new_snapshot->entries[i].forward = calloc(ptr->forward_max, sizeof(entry*)); 

		// Fill in values: 
		for (int j = 0; j < ptr->length; j++) {
			new_snapshot->entries[i].values[j] = ptr->values[j];
			if (ptr->values[j].type == ENTRY) { 
				// Search for the address of that new entry address in the snapshot: 
				for (int k = 0; k < len; k++)
					if (!strcmp(new_snapshot->entries[k].key, ptr->values[j].entry->key))
						new_snapshot->entries[i].values[j].entry = &new_snapshot->entries[k];  
			}
		}

		// Fill in Next/Prev Value in List: 
		for (int k = 0; k < len; k++) { 
			if (ptr->next != NULL)
				if (!(strcmp(new_snapshot->entries[k].key, ptr->next->key))) 
					new_snapshot->entries[i].next = &new_snapshot->entries[k]; 

			if (ptr->prev != NULL) 
				if (!(strcmp(new_snapshot->entries[k].key, ptr->prev->key)))
					new_snapshot->entries[i].prev = &new_snapshot->entries[k]; 
		}

		// Fill in backward/forward references:
		for (int k = 0; k < len; k++) { 
			for (int j = 0; j < ptr->backward_size; j++)
				if (!strcmp(ptr->backward[j]->key, new_snapshot->entries[k].key))
					new_snapshot->entries[i].backward[j] = &new_snapshot->entries[k]; 
			
			for (int j = 0; j < ptr->forward_size; j++) 
				if (!strcmp(ptr->forward[j]->key, new_snapshot->entries[k].key))
					new_snapshot->entries[i].forward[j] = &new_snapshot->entries[k];
				
		} 
		ptr = ptr->next; 
	}
	return new_snapshot; 
}

void add_snapshot(snapshot** head, snapshot* snap, int* unq_id) { 
	snapshot* last = *head; 
	snap->next = NULL; 
	if (*head == NULL) { 
		snap->prev = NULL; 
		*head = snap; 
		snap->id = *unq_id + 1; 
		(*unq_id)++; 
		return; 
	}

	while (last->next != NULL) 
		last = last->next; 
	last->next = snap; 
	snap->prev = last; 
	snap->id = (*unq_id) + 1; 
	(*unq_id)++; 
	return; 
}

void list(entry *head, snapshot* snap_head, char* line) {
	char* rem = line; 
	char* token = strtok_r(rem, " ", &rem); 
	char chr; 

	for(int i = 0; i < strlen(token); i++) { 
		chr = token[i];
		token[i] = toupper(chr);  
	}

	if (!strcmp(token, "ENTRIES\n")) { 
		entry* tmp = head; 

		if (tmp == NULL) { 
			printf("no entries\n"); 
			return; 
		}

		while (tmp->next != NULL)
			tmp = tmp->next; 
		
		while(tmp != NULL) { 
			printf("%s ", tmp->key); 
			print_entry(tmp); 
			tmp = tmp->prev; 
		}
	
	} else if (!strcmp(token, "KEYS\n")) { 
		entry* tmp = head; 

		if (tmp == NULL) {
			printf("no keys\n"); 
			return; 
		} 

		while (tmp->next != NULL)
			tmp = tmp->next;
		while(tmp != NULL) { 
			printf("%s\n", tmp->key); 
			tmp = tmp->prev; 
		}
	} else if (!strcmp(token, "SNAPSHOTS\n")) { 
		snapshot* tmp = snap_head;

		if (tmp == NULL) { 
			printf("no snapshots\n"); 
			return; 
		}

		while (tmp->next != NULL)
			tmp = tmp->next;
		while(tmp != NULL) { 
			printf("%d\n", tmp->id);  
			tmp = tmp->prev; 
		}
	}	
}

int comp(const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

void reverse(entry* head, char* line) {
	char* rem = line; 
	char* token = strtok_r(rem, " ", &rem); 
	entry* ent = get_entry(head, token); 
	if (ent == NULL) { 
		printf("no such key\n\n"); 
		return; 
	}
	if (!ent->is_simple) { 
		printf("only simple entries\n\n"); 
		return; 
	}
	element tmp; 

	if (ent->is_simple) { 
		for (int i = 0; i < ent->length / 2; i++) { 
			tmp = ent->values[i]; 
			ent->values[i] = ent->values[ent->length-i-1]; 
			ent->values[ent->length-i-1] = tmp; 
		}
	}
	printf("ok\n\n"); 
}

void sort(entry* head, char* line) { 
	char* rem = line; 
	char* token = strtok_r(rem, " ", &rem); 
	entry* ent = get_entry(head, token); 
	if (ent == NULL) { 
		printf("no such key\n\n"); 
		return; 
	}
	if (!ent->is_simple) { 
		printf("only simple entries\n\n"); 
		return; 
	}
	int values[ent->length]; 

	for (int i = 0; i < ent->length; i++) 
		values[i] = ent->values[i].value; 

	if (ent->is_simple)
		qsort(values, ent->length, sizeof(int), comp); 
	 
	for (int i = 0; i < ent->length; i++) 
		ent->values[i].value = values[i];
	printf("ok\n\n");
}

void unique(entry* head, char* line) { 
	char* rem = line; 
	char* token = strtok_r(rem, " ", &rem); 
	entry* ent = get_entry(head, token);
	if (ent == NULL) { 
		printf("no such key\n\n"); 
		return; 
	}
	if (!ent->is_simple) { 
		printf("only simple entries\n\n"); 
		return; 
	}
	element* copy = malloc(sizeof(element) * ent->length); 
	element null_element; 
	null_element.type = ENTRY; 
	int og_len = ent->length; 
	int k = 0; 

	// Copy the list 
	for(int i = 0; i < ent->length; i++)
		copy[i] = ent->values[i]; 

	// Mark the duplicates in the copied list 
	for (int i = 0; i < og_len; i++) { 
		if (i != og_len - 1 && ent->values[i].value == ent->values[i + 1].value) { 
			ent->length--; 
			copy[i] = null_element; 
		}
	}

	// Copy over non duplicates 
	FREE(ent->values);
	ent->values = calloc(ent->length, sizeof(element)); 
	for (int i = 0; i < og_len; i++) { 
		if (copy[i].type == INTEGER)
			ent->values[k++] = copy[i]; 
	}
	FREE(copy);
	printf("ok\n\n"); 
}

int calculate_min(entry* ent) { 
	int min = INT_MAX; 
	for (int i = 0; i < ent->length; i++) { 
		if (ent->values[i].type == INTEGER) { 
			if (min > ent->values[i].value) { 
				min = ent->values[i].value; 
			}
		} else if (ent->values[i].type == ENTRY) { 
			if (min > calculate_min(ent->values[i].entry)) { 
				min = calculate_min(ent->values[i].entry); 
			}
		}
	}
	return min; 
}

void min(entry* head, char* line) { 
	entry* ent = get_entry(head, line); 
	if (ent == NULL) {
		printf("no such key\n\n"); 
		return;
	}
	if (ent->length == 0) { 
		printf("nil\n\n"); 
		return; 
	}
	int min = calculate_min(ent); 
	printf("%d\n\n", min); 
}

int calculate_max(entry* ent) { 
	int max = INT_MIN; 
	for (int i = 0; i < ent->length; i++) { 
		if (ent->values[i].type == INTEGER) { 
			if (max < ent->values[i].value) { 
				max = ent->values[i].value; 
			}
		} else if (ent->values[i].type == ENTRY) { 
			if (max < calculate_max(ent->values[i].entry)) { 
				max = calculate_max(ent->values[i].entry); 
			}
		}
	}
	return max;
}

void max(entry* head, char* line) { 
	entry* ent = get_entry(head, line); 
	if (ent == NULL) {
		printf("no such key\n\n"); 
		return;
	}
	if (ent->length == 0) { 
		printf("nil\n\n"); 
		return; 
	}
	int max = calculate_max(ent); 
	printf("%d\n\n", max);
}

int calculate_sum(entry* ent) { 
	int sum = 0; 
	for (int i = 0; i < ent->length; i++) { 
		if (ent->values[i].type == INTEGER) { 
				sum += ent->values[i].value; 
		} else if (ent->values[i].type == ENTRY) { 
			sum += calculate_sum(ent->values[i].entry); 
		}
	}
	return sum;
}

void sum(entry* head, char* line) { 
	entry* ent = get_entry(head, line); 
	if (ent == NULL) {
		printf("no such key\n\n"); 
		return;
	}
	if (ent->length == 0) { 
		printf("nil\n\n"); 
		return; 
	}
	int sum = calculate_sum(ent); 
	printf("%d\n\n", sum);
}

int count_len(entry* ent) {  
	int len = 0; 
	for(int i = 0; i < ent->length; i++) { 
		if (ent->values[i].type == INTEGER)
			len++; 
		else 
			len += count_len(ent->values[i].entry); 
	}
	return len; 
}

void print_len(entry* head, char* line) { 
	entry* ent = get_entry(head, line); 
	if (ent == NULL) {
		printf("no such key\n\n"); 
		return;
	}
	if (ent->length == 0) { 
		printf("nil\n\n"); 
		return; 
	}
	printf("%d\n\n", count_len(ent)); 
}

int checkout(snapshot** snap_head, entry** head, char* line) {
	char* rem = line; 
	int id = atoi(strtok_r(rem, " ", &rem)); 
	int len = 0; 
	entry* tmp; 
	snapshot* ptr = *snap_head; 
	snapshot* requested_snapshot = NULL; 

	if (snap_head == NULL || head == NULL)
		return 0; 

	// Get requested snapshot 
	while(ptr != NULL) { 
		if (ptr->id == id) { 
			requested_snapshot = ptr; 
			break; 
		} 
		ptr = ptr->next; 
	}

	if (requested_snapshot == NULL) { 
		printf("no such snapshot\n\n"); 
		return 0; 
	}

	tmp = *head; 
	while (tmp != NULL) { 
		len++; 
		tmp = tmp->next; 
	} tmp = *head;

	while((*head) != NULL) {
		tmp = *head; 
		*head = (*head)->next;
		FREE(tmp->backward);  
		FREE(tmp->forward); 
		FREE(tmp->values); 
		FREE(tmp); 
	}

	*head = NULL; 
	// Put all entries in snapshot, without reference: 
	entry* ent_ptr = requested_snapshot->head;
	while (ent_ptr != NULL) { 
		apply_snapshot(head, ent_ptr, 0); 
		ent_ptr = ent_ptr->next;
	} ent_ptr = requested_snapshot->head;

	// Overwrite them with references: 
	while (ent_ptr != NULL) { 
		apply_snapshot(head, ent_ptr, 1); 
		ent_ptr = ent_ptr->next;
	}
	printf("ok\n\n"); 
	return 1; 
 }

 int drop(snapshot** head, char* line, int id) { 
	char* rem = line; 
	if (id == 0)
		id = atoi(strtok_r(rem, " ", &rem)); 
	snapshot* ptr = *head; 
	int found = 0; 

	// Get the requested snapshot 
	while(ptr != NULL) { 
		if (ptr->id == id) { 
			found = 1;
			break;
		} 
		else 
			ptr = ptr->next; 
	}

	if (!found) { 
		printf("no such snapshot\n\n"); 
		return 0; 
	}

	// Delete node from linked list: 
	if (*head == ptr)
		*head = ptr->next; 
	
	if (ptr->next != NULL) 
		ptr->next->prev = ptr->prev; 
	
	if (ptr->prev != NULL)
		ptr->prev->next = ptr->next; 

	entry* ent_ptr = ptr->head; 
	while (ent_ptr != NULL) { 
		FREE(ent_ptr->backward); 
		FREE(ent_ptr->forward);
		FREE(ent_ptr->values);
		ent_ptr = ent_ptr->next;
	}
	if (ptr->len != 0)
		FREE(ptr->entries); 
	FREE(ptr); 
	return 1; 
}

void rollback(snapshot** snap_head, entry** head, char* line) { 
	if (!checkout(snap_head, head, line))
		return;
	char* rem = line; 
	int id = atoi(strtok_r(rem, " ", &rem)); 
	snapshot* tmp = *snap_head;  

	// Store all the ids of the snapshots to be deleted
	// Future me, we did this because it was deleting a snapshot, then the list would get messed up 
	// because it would skip over the next id in tmp: 

	// Count how many snapshots there are 
	int len = 0; 
	while(tmp != NULL) { 
		tmp = tmp->next;
		len++; 
	} tmp = *snap_head;

	int* ids_to_be_deleted = calloc(len, sizeof(int)); 
	int i = 0;
	while (tmp != NULL) { 
		if (tmp->next != NULL) { 
			if (tmp->next->id > id) { 
				ids_to_be_deleted[i++] = tmp->next->id;
			}
		}
		tmp = tmp->next;
	}

	for (int j = 0; j < i; j++)
		drop(snap_head, line, ids_to_be_deleted[j]); 
	FREE(ids_to_be_deleted);
	return; 
}

void command_bye(entry* head, snapshot* snap_head) {
	entry* tmp; 
	snapshot* temp; 
	entry* ent_ptr;

	while (head != NULL) { 
		tmp = head;
		head = head->next;
		FREE(tmp->backward); 
		FREE(tmp->forward); 
		FREE(tmp->values); 
		FREE(tmp); 
	}

	while (snap_head != NULL) { 
		temp = snap_head; 
		ent_ptr = snap_head->head;
		snap_head = snap_head->next; 
		while(ent_ptr != NULL) { 
			FREE(ent_ptr->forward);
			FREE(ent_ptr->backward);
			FREE(ent_ptr->values);
			ent_ptr = ent_ptr->next;
		}
		if (temp->len != 0)
			FREE(temp->entries); 
		FREE(temp); 
	}
	printf("bye\n");
}

void command_help() {
	printf("%s\n", HELP);
}

int main(void) {
	char chr; 
	int uniq_snapshot_id = 0;  
	snapshot* snap; 
	snapshot* head_snapshot = NULL; 
	entry* ent; 
	entry* head = NULL; 

	char line[MAX_LINE];

	while (true) {
		printf("> ");

		if (NULL == fgets(line, MAX_LINE, stdin)) {
			printf("\n");
			command_bye(head, head_snapshot);
			return 0;
		}

		// Isolate command: 
		char* command = strtok(line, " "); 

		// Make command capital: 
		for(int i = 0; i < strlen(command); i ++) { 
			chr = command[i];
			command[i] = toupper(chr);  
		}

		//Isolate keys and values, if any: 
		char* line1 = strtok(NULL, "");
		char line2[MAX_LINE]; 
		if (line1 != NULL)
			strcpy(line2, line1); 

		// Database Commands: 
		if (!(strcmp(command, "SET"))) { 
			ent = get_entry(head, line1); 
			if (ent != NULL) { 
				update_entry(head, ent, line2); 
			} else { 
				ent = create_entry(line2, head, 1); 
				if (ent->is_successful) { 
					append_to_entries(&head, ent);
					printf("ok\n\n"); 
				}
			} 
		} 
		else if (!(strcmp(command, "GET"))) {
			ent = get_entry(head, line1); 
			if (ent != NULL) { 
				print_entry(ent); 
				printf("\n"); 
			}
			else { 
				printf("no such key\n\n");
			}
		}
		else if (!(strcmp(command, "PUSH"))) { 
			ent = get_entry(head, line1); 
			if (ent != NULL) { 
				push(head, ent, line2);
			}  
			else 
				printf("no such key\n\n"); 
		} else if (!(strcmp(command, "APPEND"))) { 
			ent = get_entry(head, line1); 
			if (ent != NULL)
				append(head, ent, line2);
			else 
				printf("no such key\n\n"); 
		} else if (!(strcmp(command, "BACKWARD"))) { 
			print_backward(head, line1); 
		} else if (!(strcmp(command, "FORWARD"))) { 
			print_forward(head, line1); 
		} else if (!(strcmp(command, "TYPE"))) { 
			type(head, line1); 
		} else if (!(strcmp(command, "HELP"))) { 
			command_help(); 
		} else if (!strcmp(command, "BYE\n")) {
			command_bye(head, head_snapshot); 
			break; 
		} else if (!(strcmp(command, "DEL"))) { 
			if (delete(&head, line1, NULL)) 
				printf("ok\n\n"); 
		} else if (!(strcmp(command, "PICK"))) { 
			pick(head, line1);
		} else if (!(strcmp(command, "PLUCK"))) { 
			pluck(head, line1); 
		} else if (!(strcmp(command, "POP"))) { 
			pop(head, line1); 
		} else if (!(strcmp(command, "SNAPSHOT\n"))) { 
			snap = create_snapshot(head); 
			add_snapshot(&head_snapshot, snap, &uniq_snapshot_id); 
			printf("saved as snapshot %d\n\n", snap->id); 
		} else if (!(strcmp(command, "CHECKOUT"))) { 
			checkout(&head_snapshot, &head, line1); 
		} else if (!(strcmp(command, "DROP"))) {
			if (drop(&head_snapshot, line1, 0))
				printf("ok\n\n"); 
		} else if (!(strcmp(command, "ROLLBACK"))) { 
			rollback(&head_snapshot, &head, line1);
		} else if (!strcmp(command, "PURGE")) { 
			purge(&head, head_snapshot, line1); 
		} else if (!strcmp(command, "LIST")) {
			list(head, head_snapshot, line1); 
			printf("\n"); 
		} else if (!strcmp(command, "REV")) { 
			reverse(head, line1); 
		} else if (!strcmp(command, "SORT")) { 
			sort(head, line1); 
		} else if (!strcmp(command, "UNIQ")) { 
			unique(head, line1); 
		} else if (!strcmp(command, "LEN")) { 
			print_len(head, line1); 
		} else if (!strcmp(command, "MIN")) { 
			min(head, line1); 
		} else if (!strcmp(command, "MAX")) { 
			max(head, line1); 
		} else if (!strcmp(command, "SUM")) { 
			sum(head, line1); 
		} else if (!strcmp(command, "HELP\n")) { 
			command_help(); 
		}
	}
	return 0;
}