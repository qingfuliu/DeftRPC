//
// Created by lqf on 23-4-26.
//

#ifndef DEFTRPC_CONFIG_H
#define DEFTRPC_CONFIG_H

namespace clsn {

#define DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME SerializeFunc
#define DEFTRPC_DESERIALIZE_INPUT_FUNCNAME DeSerializeFunc

#define DEFTRPC_DESERIALIZE_LOAD_AND_CONSTRUCT_FUNCNANE LoadAndConstruct

using address_type = uint64_t;
using id_type = uint64_t;
constexpr id_type ptr_id_mask = 0x8000000000000000;
}  // namespace clsn

#endif  // DEFTRPC_CONFIG_H
