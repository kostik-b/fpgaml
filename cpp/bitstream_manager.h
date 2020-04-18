// Copyright QUB 2018

#ifndef es_bitstream_manager_hh
#define es_bitstream_manager_hh

#include <string>
#include <map>
#include <vector>
#include "fpgaml_rc.h"

#include "wee_config.h"

using namespace std;

namespace FPGAML
{

typedef pair<char*, int> pair_f;

struct bitstream_s
{
  string            resource_string;
  vector<unsigned>  offset_address;
  string            target;
  unsigned          num_accs;
  unsigned          wgroup_size;
  unsigned          local_cost;
  unsigned          remote_cost;
  string            bitstream;
  unsigned          bs_length;
};

typedef std::vector<string>::iterator                 vec_iter;
typedef std::map<string, bitstream_s>::const_iterator map_iter;

class BitstreamManager
{
public:
  BitstreamManager (WeeConfig& config);
  BitstreamManager (const BitstreamManager& copy) = delete;
  BitstreamManager& operator= (const BitstreamManager& rhs) = delete;

  //get the bitstream with the given kernel_name and num_islands ???
  FPGAML::RC get_bit_stream (const string& fname, string& bitstream);

  //get the resource string with the given kernel_name
  FPGAML::RC get_resource_string(const string& fname, string& resource_string);

  //get the list of offset address with the given kernel_name
  FPGAML::RC get_offset_addresses (const string& fname, vector<unsigned>& offset_addresses);

  //get the target with the given kernel_name
  FPGAML::RC get_target (const string& fname, string& target);

  FPGAML::RC get_num_accs (const string& fname, unsigned& num_accs);

  FPGAML::RC get_wg_size  (const string& fname, unsigned& wg_size);

  FPGAML::RC get_local_cost (const string& fname, unsigned& cost);

  FPGAML::RC get_remote_cost (const string& fname, unsigned& cost);

  //get the bitstream length with the given kernel_name
  FPGAML::RC get_bs_length (const string& fname, unsigned& bs_length);

  // deprecated as "get_map_iterators" supercedes it
  pair<vec_iter, vec_iter>  
             get_xml_files_list () { 
                                     return std::make_pair(m_xml_files_list.begin (),
                                                           m_xml_files_list.end ());
                                   }

  pair<map_iter, map_iter>
            get_map_iterators ()
                                   {
                                     return std::make_pair(m_kernel_bitstream_struct_map.begin (),
                                                           m_kernel_bitstream_struct_map.end ());
                                   }

  ~BitstreamManager () ;

private:
  map<string, bitstream_s>      m_kernel_bitstream_struct_map;
  map<string, pair_f > 	        m_kernel_mmap_ptr_map;
  string                        m_working_dir_path;
  bool                          m_enabled_mmap;
  
  vector<string>                m_xml_files_list;

  //read all xml files in the given directory and parse all information to maps
  //name of the given directory can be read from configuration file or provided from environment variable
  FPGAML::RC read_xml_files ();

  FPGAML::RC get_xml_files_list (vector<string>& xml_files_vec);

  FPGAML::RC extract_xml_file (const string& xml_file);

}; // class BitstreamManager

} // namespace

#endif
