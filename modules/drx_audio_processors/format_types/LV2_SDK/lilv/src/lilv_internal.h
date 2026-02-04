/*
  Copyright 2007-2019 David Robillard <d@drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef LILV_INTERNAL_H
#define LILV_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lilv_config.h" // IWYU pragma: keep

#include "lilv/lilv.h"
#include "lv2/core/lv2.h"
#include "serd/serd.h"
#include "sord/sord.h"
#include "zix/tree.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
#  include <direct.h>
#  include <stdio.h>
#  include <windows.h>
#  define dlopen(path, flags) LoadLibrary(path)
#  define dlclose(lib) FreeLibrary((HMODULE)lib)
#  ifdef _MSC_VER
#    define __func__ __FUNCTION__
#    ifndef snprintf
#      define snprintf _snprintf
#    endif
#  endif
#  ifndef INFINITY
#    define INFINITY DBL_MAX + DBL_MAX
#  endif
#  ifndef NAN
#    define NAN INFINITY - INFINITY
#  endif
static inline tukk
dlerror(z0)
{
  return "Unknown error";
}
#else
#  include <dlfcn.h>
#endif

#ifdef LILV_DYN_MANIFEST
#  include "lv2/dynmanifest/dynmanifest.h"
#endif

/*
 *
 * Types
 *
 */

typedef z0 LilvCollection;

struct LilvPortImpl {
  LilvNode*  node;    ///< RDF node
  u32   index;   ///< lv2:index
  LilvNode*  symbol;  ///< lv2:symbol
  LilvNodes* classes; ///< rdf:type
};

typedef struct LilvSpecImpl {
  SordNode*            spec;
  SordNode*            bundle;
  LilvNodes*           data_uris;
  struct LilvSpecImpl* next;
} LilvSpec;

/**
   Header of an LilvPlugin, LilvPluginClass, or LilvUI.
   Any of these structs may be safely casted to LilvHeader, which is used to
   implement collections using the same comparator.
*/
struct LilvHeader {
  LilvWorld* world;
  LilvNode*  uri;
};

#ifdef LILV_DYN_MANIFEST
typedef struct {
  LilvNode*               bundle;
  uk                   lib;
  LV2_Dyn_Manifest_Handle handle;
  u32                refs;
} LilvDynManifest;
#endif

typedef struct {
  LilvWorld*                world;
  LilvNode*                 uri;
  tuk                     bundle_path;
  uk                     lib;
  LV2_Descriptor_Function   lv2_descriptor;
  const LV2_Lib_Descriptor* desc;
  u32                  refs;
} LilvLib;

struct LilvPluginImpl {
  LilvWorld* world;
  LilvNode*  plugin_uri;
  LilvNode*  bundle_uri; ///< Bundle plugin was loaded from
  LilvNode*  binary_uri; ///< lv2:binary
#ifdef LILV_DYN_MANIFEST
  LilvDynManifest* dynmanifest;
#endif
  const LilvPluginClass* plugin_class;
  LilvNodes*             data_uris; ///< rdfs::seeAlso
  LilvPort**             ports;
  u32               num_ports;
  b8                   loaded;
  b8                   parse_errors;
  b8                   replaced;
};

struct LilvPluginClassImpl {
  LilvWorld* world;
  LilvNode*  uri;
  LilvNode*  parent_uri;
  LilvNode*  label;
};

struct LilvInstancePimpl {
  LilvWorld* world;
  LilvLib*   lib;
};

typedef struct {
  b8  dyn_manifest;
  b8  filter_language;
  tuk lv2_path;
} LilvOptions;

struct LilvWorldImpl {
  SordWorld*         world;
  SordModel*         model;
  SerdReader*        reader;
  u32           n_read_files;
  LilvPluginClass*   lv2_plugin_class;
  LilvPluginClasses* plugin_classes;
  LilvSpec*          specs;
  LilvPlugins*       plugins;
  LilvPlugins*       zombies;
  LilvNodes*         loaded_files;
  ZixTree*           libs;
  struct {
    SordNode* dc_replaces;
    SordNode* dman_DynManifest;
    SordNode* doap_name;
    SordNode* lv2_Plugin;
    SordNode* lv2_Specification;
    SordNode* lv2_appliesTo;
    SordNode* lv2_binary;
    SordNode* lv2_default;
    SordNode* lv2_designation;
    SordNode* lv2_extensionData;
    SordNode* lv2_index;
    SordNode* lv2_latency;
    SordNode* lv2_maximum;
    SordNode* lv2_microVersion;
    SordNode* lv2_minimum;
    SordNode* lv2_minorVersion;
    SordNode* lv2_name;
    SordNode* lv2_optionalFeature;
    SordNode* lv2_port;
    SordNode* lv2_portProperty;
    SordNode* lv2_reportsLatency;
    SordNode* lv2_requiredFeature;
    SordNode* lv2_symbol;
    SordNode* lv2_prototype;
    SordNode* owl_Ontology;
    SordNode* pset_value;
    SordNode* rdf_a;
    SordNode* rdf_value;
    SordNode* rdfs_Class;
    SordNode* rdfs_label;
    SordNode* rdfs_seeAlso;
    SordNode* rdfs_subClassOf;
    SordNode* xsd_base64Binary;
    SordNode* xsd_boolean;
    SordNode* xsd_decimal;
    SordNode* xsd_double;
    SordNode* xsd_integer;
    SordNode* null_uri;
  } uris;
  LilvOptions opt;
};

typedef enum {
  LILV_VALUE_URI,
  LILV_VALUE_STRING,
  LILV_VALUE_INT,
  LILV_VALUE_FLOAT,
  LILV_VALUE_BOOL,
  LILV_VALUE_BLANK,
  LILV_VALUE_BLOB
} LilvNodeType;

struct LilvNodeImpl {
  LilvWorld*   world;
  SordNode*    node;
  LilvNodeType type;
  union {
    i32   int_val;
    f32 float_val;
    b8  bool_val;
  } val;
};

struct LilvScalePointImpl {
  LilvNode* value;
  LilvNode* label;
};

struct LilvUIImpl {
  LilvWorld* world;
  LilvNode*  uri;
  LilvNode*  bundle_uri;
  LilvNode*  binary_uri;
  LilvNodes* classes;
};

typedef struct LilvVersion {
  i32 minor;
  i32 micro;
} LilvVersion;

/*
 *
 * Functions
 *
 */

LilvPort*
lilv_port_new(LilvWorld*      world,
              const SordNode* node,
              u32        index,
              tukk     symbol);
z0
lilv_port_free(const LilvPlugin* plugin, LilvPort* port);

LilvPlugin*
lilv_plugin_new(LilvWorld* world, LilvNode* uri, LilvNode* bundle_uri);

z0
lilv_plugin_clear(LilvPlugin* plugin, LilvNode* bundle_uri);

z0
lilv_plugin_load_if_necessary(const LilvPlugin* plugin);

z0
lilv_plugin_free(LilvPlugin* plugin);

LilvNode*
lilv_plugin_get_unique(const LilvPlugin* plugin,
                       const SordNode*   subject,
                       const SordNode*   predicate);

z0
lilv_collection_free(LilvCollection* collection);

u32
lilv_collection_size(const LilvCollection* collection);

LilvIter*
lilv_collection_begin(const LilvCollection* collection);

uk
lilv_collection_get(const LilvCollection* collection, const LilvIter* i);

LilvPluginClass*
lilv_plugin_class_new(LilvWorld*      world,
                      const SordNode* parent_node,
                      const SordNode* uri,
                      tukk     label);

z0
lilv_plugin_class_free(LilvPluginClass* plugin_class);

LilvLib*
lilv_lib_open(LilvWorld*                world,
              const LilvNode*           uri,
              tukk               bundle_path,
              const LV2_Feature* const* features);

const LV2_Descriptor*
lilv_lib_get_plugin(LilvLib* lib, u32 index);

z0
lilv_lib_close(LilvLib* lib);

LilvNodes*
lilv_nodes_new(z0);

LilvPlugins*
lilv_plugins_new(z0);

LilvScalePoints*
lilv_scale_points_new(z0);

LilvPluginClasses*
lilv_plugin_classes_new(z0);

LilvUIs*
lilv_uis_new(z0);

LilvNode*
lilv_world_get_manifest_uri(LilvWorld* world, const LilvNode* bundle_uri);

u8k*
lilv_world_blank_node_prefix(LilvWorld* world);

SerdStatus
lilv_world_load_file(LilvWorld* world, SerdReader* reader, const LilvNode* uri);

SerdStatus
lilv_world_load_graph(LilvWorld* world, SordNode* graph, const LilvNode* uri);

LilvUI*
lilv_ui_new(LilvWorld* world,
            LilvNode*  uri,
            LilvNode*  type_uri,
            LilvNode*  binary_uri);

z0
lilv_ui_free(LilvUI* ui);

LilvNode*
lilv_node_new(LilvWorld* world, LilvNodeType type, tukk str);

LilvNode*
lilv_node_new_from_node(LilvWorld* world, const SordNode* node);

i32
lilv_header_compare_by_uri(ukk a, ukk b, ukk user_data);

i32
lilv_lib_compare(ukk a, ukk b, ukk user_data);

i32
lilv_ptr_cmp(ukk a, ukk b, ukk user_data);

i32
lilv_resource_node_cmp(ukk a, ukk b, ukk user_data);

static inline i32
lilv_version_cmp(const LilvVersion* a, const LilvVersion* b)
{
  if (a->minor == b->minor && a->micro == b->micro) {
    return 0;
  }

  if ((a->minor < b->minor) || (a->minor == b->minor && a->micro < b->micro)) {
    return -1;
  }

  return 1;
}

struct LilvHeader*
lilv_collection_get_by_uri(const ZixTree* seq, const LilvNode* uri);

LilvScalePoint*
lilv_scale_point_new(LilvNode* value, LilvNode* label);

z0
lilv_scale_point_free(LilvScalePoint* point);

SordIter*
lilv_world_query_internal(LilvWorld*      world,
                          const SordNode* subject,
                          const SordNode* predicate,
                          const SordNode* object);

b8
lilv_world_ask_internal(LilvWorld*      world,
                        const SordNode* subject,
                        const SordNode* predicate,
                        const SordNode* object);

LilvNodes*
lilv_world_find_nodes_internal(LilvWorld*      world,
                               const SordNode* subject,
                               const SordNode* predicate,
                               const SordNode* object);

SordModel*
lilv_world_filter_model(LilvWorld*      world,
                        SordModel*      model,
                        const SordNode* subject,
                        const SordNode* predicate,
                        const SordNode* object,
                        const SordNode* graph);

#define FOREACH_MATCH(iter) for (; !sord_iter_end(iter); sord_iter_next(iter))

LilvNodes*
lilv_nodes_from_stream_objects(LilvWorld*    world,
                               SordIter*     stream,
                               SordQuadIndex field);

tuk
lilv_strjoin(tukk first, ...);

tuk
lilv_strdup(tukk str);

tuk
lilv_get_lang(z0);

tuk
lilv_expand(tukk path);

tuk
lilv_get_latest_copy(tukk path, tukk copy_path);

tuk
lilv_find_free_path(tukk in_path,
                    b8 (*exists)(tukk, ukk),
                    ukk user_data);

typedef z0 (*LilvVoidFunc)(z0);

/** dlsym wrapper to return a function pointer (without annoying warning) */
static inline LilvVoidFunc
lilv_dlfunc(uk handle, tukk symbol)
{
#ifdef _WIN32
  return (LilvVoidFunc)GetProcAddress((HMODULE)handle, symbol);
#else
  typedef LilvVoidFunc (*VoidFuncGetter)(uk, tukk);
  VoidFuncGetter dlfunc = (VoidFuncGetter)dlsym;
  return dlfunc(handle, symbol);
#endif
}

#ifdef LILV_DYN_MANIFEST
static const LV2_Feature* const dman_features = {NULL};

z0
lilv_dynmanifest_free(LilvDynManifest* dynmanifest);
#endif

#define LILV_ERROR(str) fprintf(stderr, "%s(): error: " str, __func__)
#define LILV_ERRORF(fmt, ...) \
  fprintf(stderr, "%s(): error: " fmt, __func__, __VA_ARGS__)
#define LILV_WARN(str) fprintf(stderr, "%s(): warning: " str, __func__)
#define LILV_WARNF(fmt, ...) \
  fprintf(stderr, "%s(): warning: " fmt, __func__, __VA_ARGS__)
#define LILV_NOTE(str) fprintf(stderr, "%s(): note: " str, __func__)
#define LILV_NOTEF(fmt, ...) \
  fprintf(stderr, "%s(): note: " fmt, __func__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* LILV_INTERNAL_H */
