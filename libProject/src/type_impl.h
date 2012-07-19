#include "type.h"
#include "utils.h"

static o_bool TypeIsPrimitive(Type* t) {
	return t->fields == NULL;
}

static uword findLargestAlignment(uword largest,
                                  Field* field) {
    ArrayInfo* aInfo;
    Box* box;
    uword i, align;

    if(TypeIsPrimitive(field->type)) {
		align = field->type->alignment != 0 ? field->type->alignment : field->type->size;
		if(align > largest)
			largest = align;
    }
	else {
		if(field->type->alignment != 0) {
			if(field->type->alignment > largest) {
				largest = field->type->alignment;
			}
		}
		else {
            // box and aInfo should always be valid because if there are no fields
            // then type->fields is set to NULL and IsPrimitive is true
            box = BoxGetBox(field->type->fields);
            aInfo = BoxGetArrayInfo(box);
			for(i = 0; i < aInfo->num_elements; ++i) {
				largest = findLargestAlignment(largest, &field->type->fields[i]);
			}
		}
	}
    
    return largest;
}

static void TypeCreate(Type* type,
                       uword alignment,
                       struct Symbol* name,
                       struct Field* fields) {
    ArrayInfo* aInfo = BoxGetArrayInfo(BoxGetBox(fields));
    _TypeCreate(type, alignment, name, fields, aInfo->num_elements);
}

static void _TypeCreate(Type* type,
                       uword alignment,
                       struct Symbol* name,
                       struct Field* fields,
                        uword numFields) {
    uword largestMember;
    uword i;
    uword align;

    type->isArray = o_false;
    type->name = name;
    type->alignment = alignment;
    type->fields = fields;
    type->size = 0;

    largestMember = 0;
    
    for(i = 0; i < numFields; ++i) {
        if(fields[i].ptr) {
            type->size = alignOffset(type->size, sizeof(pointer));
            fields[i].offset = type->size;
            type->size += sizeof(pointer);
            if(largestMember < sizeof(pointer))
                largestMember = sizeof(pointer);
        } else { // inline
            if(TypeIsPrimitive(fields[i].type)) {
                align = fields[i].type->alignment != 0 ? fields[i].type->alignment : fields[i].type->size;
            } else {
                // Align composite types on pointer if there is no explicit alignment
                align = fields[i].type->alignment != 0 ? fields[i].type->alignment : sizeof(void*);
            }
            type->size = alignOffset(type->size, align);
            fields[i].offset = type->size;
            type->size += fields[i].type->size;
            largestMember = findLargestAlignment(largestMember, &fields[i]);
        }
    }
    
    type->size = nextLargerMultiple(largestMember, type->size);
}

static void FieldCreate(Field* field,
                        struct Symbol* name,
                        struct Type* type,
                        o_bool ptr) {
    field->name = name;
    field->type = type;
    field->ptr = ptr;
    field->offset = 0;
}

static void ArrayTypeCreate(ArrayType* arrType,
                            Type* type,
                            uword size) {
    // TODO
}







