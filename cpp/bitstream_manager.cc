// Copyright QUB 2018


#include <dirent.h>
#include <iostream>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdlib>
#include "pugixml.hpp"
#include "bitstream_manager.h"
#include "wee_config.h"
#include "centrum.h"

FPGAML::BitstreamManager::BitstreamManager(WeeConfig& config)
{
  const char* xml_path = NULL;
  config.get_value_for_key ("BitstreamsPath", xml_path);

  if (xml_path)
  {
    m_working_dir_path = xml_path;
  }

  m_enabled_mmap = false;

  config.get_value_for_key ("EnableBitstreamMmap", m_enabled_mmap);

  read_xml_files();
}

//get the bitstream with the given kernel_name and num_islands ???
FPGAML::RC FPGAML::BitstreamManager::get_bit_stream (const string& fname, string& bstream)
{
  //cout << "get_bit_stream" << endl;

  char* filePtr = NULL;


  if(m_enabled_mmap)
  {
    clock_t start = clock ();
    map<string, pair_f>::iterator it = m_kernel_mmap_ptr_map.find(fname);
    if(it!= m_kernel_mmap_ptr_map.end())
    {
      filePtr = it->second.first;
    }
    else
    {
      //cout << "Could not find " << fname << " in m_kernel_mmap_ptr_map." << endl;
      return RC::ML_ERROR;
    }


    pugi::xml_document doc;

    if (! doc.load_string(filePtr))
    {
      //cout << "Could not load from file pointer." << endl;
      return RC::ML_ERROR;
    }
    //extract bitstream
    pugi::xml_node bs = doc.child("data").child("body").child("bitstream");
    //cout << xml_file << bitstream.name() << " : " << bitstream.child_value() << endl;
    bstream = string(bs.child_value());

    //cout << "Estimated time : " << float(clock () - start)/CLOCKS_PER_SEC << endl;
    return RC::ML_SUCCESS;

  }
  else
  {
    clock_t start = clock ();

    map<string, bitstream_s>::iterator it = m_kernel_bitstream_struct_map.find(fname);

    // cout << "Estimated time : " << std::fixed << float(clock () - start)/CLOCKS_PER_SEC << endl;
    if(it!= m_kernel_bitstream_struct_map.end())
    {
      bstream = ((struct bitstream_s)(it->second)).bitstream;
      return RC::ML_SUCCESS;
    }
    else
      return RC::ML_ERROR;
  }

}

//get the resource string with the given kernel_name
FPGAML::RC FPGAML::BitstreamManager::get_resource_string(const string& fname, string& resource_string)
{
  //cout << "get_resource_string" << endl;
  map<string, bitstream_s>::iterator it = m_kernel_bitstream_struct_map.find(fname);
  if(it!= m_kernel_bitstream_struct_map.end())
  {
    resource_string = ((struct bitstream_s)(it->second)).resource_string;
    return RC::ML_SUCCESS;
  }
  else
    return RC::ML_ERROR;
}

//get the list of offset address with the given kernel_name
FPGAML::RC FPGAML::BitstreamManager::get_offset_addresses (const string& fname, vector<unsigned>& offset_addresses)
{
  // cout << "get_offset_address" << endl;
  map<string, bitstream_s>::iterator it = m_kernel_bitstream_struct_map.find(fname);
  if(it!= m_kernel_bitstream_struct_map.end())
  {
    offset_addresses = ((struct bitstream_s)(it->second)).offset_address;
    return RC::ML_SUCCESS;
  }
  else
    return RC::ML_ERROR;
}

//get the target with the given kernel_name
FPGAML::RC FPGAML::BitstreamManager::get_target (const string& fname, string& target)
{
  // cout << "get_target" << endl;
  map<string, bitstream_s>::iterator it = m_kernel_bitstream_struct_map.find(fname);
  if(it!= m_kernel_bitstream_struct_map.end())
  {
    target = ((struct bitstream_s)(it->second)).target;
    return RC::ML_SUCCESS;
  }
  else
    return RC::ML_ERROR;
}

FPGAML::RC FPGAML::BitstreamManager::get_num_accs (const string& fname, unsigned& num_accs)
{
  // cout << "get_target" << endl;
  map<string, bitstream_s>::iterator it = m_kernel_bitstream_struct_map.find(fname);
  if(it!= m_kernel_bitstream_struct_map.end())
  {
    num_accs = ((struct bitstream_s)(it->second)).num_accs;
    return RC::ML_SUCCESS;
  }
  else
    return RC::ML_ERROR;
}

FPGAML::RC FPGAML::BitstreamManager::get_wg_size (const string& fname, unsigned& wg_size)
{
  // cout << "get_target" << endl;
  map<string, bitstream_s>::iterator it = m_kernel_bitstream_struct_map.find(fname);
  if(it!= m_kernel_bitstream_struct_map.end())
  {
    wg_size = ((struct bitstream_s)(it->second)).wgroup_size;
    return RC::ML_SUCCESS;
  }
  else
    return RC::ML_ERROR;
}

FPGAML::RC FPGAML::BitstreamManager::get_local_cost (const string& fname, unsigned& cost)
{
  // cout << "get_target" << endl;
  map<string, bitstream_s>::iterator it = m_kernel_bitstream_struct_map.find(fname);
  if(it!= m_kernel_bitstream_struct_map.end())
  {
    cost = ((struct bitstream_s)(it->second)).local_cost;
    return RC::ML_SUCCESS;
  }
  else
    return RC::ML_ERROR;
}

FPGAML::RC FPGAML::BitstreamManager::get_remote_cost (const string& fname, unsigned& cost)
{
  // cout << "get_target" << endl;
  map<string, bitstream_s>::iterator it = m_kernel_bitstream_struct_map.find(fname);
  if(it!= m_kernel_bitstream_struct_map.end())
  {
    cost = ((struct bitstream_s)(it->second)).remote_cost;
    return RC::ML_SUCCESS;
  }
  else
    return RC::ML_ERROR;
}

//get the bitstream length with the given kernel_name
FPGAML::RC FPGAML::BitstreamManager::get_bs_length (const string& fname, unsigned& bs_length)
{
  //cout << "get_bs_length" << endl;
  map<string, bitstream_s>::iterator it = m_kernel_bitstream_struct_map.find(fname);
  if(it!= m_kernel_bitstream_struct_map.end())
  {
    bs_length = ((struct bitstream_s)(it->second)).bs_length;
    return RC::ML_SUCCESS;
  }
  else
    return RC::ML_ERROR;
}

FPGAML::RC FPGAML::BitstreamManager::get_xml_files_list (vector<string>& xml_files_vec)
{
  // cout << "get_xml_files_list_mmap" << endl;
  DIR* dir = opendir (m_working_dir_path.c_str());
  if (dir)
  {
    //cout << "Working directory : " << m_working_dir_path << endl;
    struct dirent* current_file = NULL;
    errno = 0;
    while ((current_file = readdir (dir)) != NULL)
    {
      string stmp = string(current_file->d_name);
      //if special directories and hidden files
      if(!stmp.compare(".") || !stmp.compare("..") || !stmp.compare(0,1,".")) continue;
      //otherwise, get the file extension
      string sub_stmp = stmp.substr (stmp.size() - 4, 4);
      //if the file extension is .xml
      if(!sub_stmp.compare(".xml")){
        //cout << "found an .xml file: " << stmp << endl;
        xml_files_vec.push_back(stmp);

        //////////////////
        if (m_enabled_mmap)
        {
          string full_xml_path = m_working_dir_path + "/" + stmp;
          int fd = open (full_xml_path.c_str(), O_RDONLY);
          if (fd < 0 )
          {
            //cout << "Could not open xml file for reading." << endl;
            perror("open");
            return RC::ML_ERROR;
          }
          struct stat fileStats;
          if (fstat (fd, &fileStats) < 0)
          {
            //cout << "Could not stat the xml file." << endl;
            perror("fstat");
            close (fd);
            return RC::ML_ERROR;
          }
          char* filePtr = (char*) mmap(NULL, fileStats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
          if (filePtr == MAP_FAILED)
          {
            //cout << "Could not mmmap file." << endl;
            perror ("mmap");
            close (fd);
            return RC::ML_ERROR;
          }
          m_kernel_mmap_ptr_map[stmp] = make_pair(filePtr, fileStats.st_size);

          close (fd);
        }


      }
    }
    closedir (dir);
    return RC::ML_SUCCESS;
  }
  else
  {
    //cout << "Could not find the working directory path." << endl;
    return RC::ML_ERROR;
  }
}


FPGAML::RC FPGAML::BitstreamManager::extract_xml_file (const string& xml_file)
{
  //cout << "extract_xml_file_mmap" << endl;
  char* filePtr = NULL;

  if(m_enabled_mmap)
  {

    map<string, pair_f>::iterator it = m_kernel_mmap_ptr_map.find(xml_file);
    if(it!= m_kernel_mmap_ptr_map.end())
    {
      filePtr = it->second.first;
    }
    else
    {
      //cout << "Could not find " << xml_file << " in m_kernel_mmap_ptr_map." << endl;
      return RC::ML_ERROR;
    }
  }

  pugi::xml_document doc;
  if(!m_enabled_mmap)
  {
    string full_xml_path = m_working_dir_path + "/" + xml_file;
    if (!doc.load_file(full_xml_path.c_str()))
    {
      //cout << "Could not load from xml file." << endl;
      return RC::ML_ERROR;
    }
  }
  else
  {
    if (! doc.load_string(filePtr))
    {
      //cout << "Could not load from file pointer." << endl;
      return RC::ML_ERROR;
    }
  }

  //cout << "Extracting file : " << xml_file << endl;
  /*for(int i=0 ; i< 10; i++)
    cout << filePtr[i];
  cout << endl;
   */
  // tag::code[]
  int n = 0;
  struct bitstream_s bs_s;

  pugi::xml_node target = doc.child("data").child("header").child("target");
  //cout << target.name() << " " << target.child_value() << endl;
  bs_s.target = string(target.child_value());


  pugi::xml_node num_accs = doc.child("data").child("header").child("num_accelerators");
  //cout << target.name() << " " << target.child_value() << endl;
  bs_s.num_accs = atoi(num_accs.child_value());

  pugi::xml_node wg_size_node = doc.child("data").child("header").child("workgroup_size");
  //cout << target.name() << " " << target.child_value() << endl;
  bs_s.wgroup_size = atoi(wg_size_node.child_value());

  pugi::xml_node local_cost_node = doc.child("data").child("header").child("local_cost");
  bs_s.local_cost = atoi(local_cost_node.child_value());

  pugi::xml_node remote_cost_node = doc.child("data").child("header").child("remote_cost");
  bs_s.remote_cost = atoi(remote_cost_node.child_value());

  pugi::xml_node rs = doc.child("data").child("header").child("resource-string");
  //cout << rs.name() << " " << rs.child_value() << endl;
  bs_s.resource_string = string(rs.child_value());

  pugi::xml_node n_args = doc.child("data").child("header").child("n_args");
  //cout << n_args.name() << " " << n_args.child_value() << endl;
  n = atoi(n_args.child_value());

  //extract offset addresses
  for(int i=0; i<n; i++)
  {
    pugi::xml_node off_add = doc.child("data").child("header").child((string("offset_address_") + to_string(i)).c_str());
    bs_s.offset_address.push_back(strtoul(off_add.child_value(), NULL, 16));
  }

  //extract bitstream's length
  pugi::xml_node length = doc.child("data").child("body").child("length");
  bs_s.bs_length = atoi(length.child_value());

  if (!m_enabled_mmap)
  {
    //extract bitstream
    pugi::xml_node bitstream = doc.child("data").child("body").child("bitstream");
    //cout << xml_file << bitstream.name() << " : " << bitstream.child_value() << endl;
    bs_s.bitstream = string(bitstream.child_value());
  }

  m_kernel_bitstream_struct_map[xml_file] = bs_s;


  return RC::ML_SUCCESS;
}

FPGAML::RC FPGAML::BitstreamManager::read_xml_files ()
{
  //cout << "read_xml_files" << endl;

  //get list of xml files and put them into xml_files_vec
  if(get_xml_files_list (m_xml_files_list) == RC::ML_SUCCESS)
  {   //extract xml files and store data to maps
    clock_t start = clock ();
    for(unsigned int i=0; i < m_xml_files_list.size(); i++)
    {
      if (extract_xml_file (m_xml_files_list[i]) == RC::ML_ERROR)
        return RC::ML_ERROR;
    }
    //cout << "Estimated time : " << float(clock () - start)/CLOCKS_PER_SEC << endl;
    return RC::ML_SUCCESS;
  }
  else
    return RC::ML_ERROR;

}

FPGAML::BitstreamManager::~BitstreamManager ()
{
  if (m_enabled_mmap)
  {
    if (!m_kernel_mmap_ptr_map.empty())
    {
      map<string, pair_f>::iterator it = m_kernel_mmap_ptr_map.begin();
      for(;it != m_kernel_mmap_ptr_map.end(); ++it){
        if (munmap (it->second.first, it->second.second) < 0)
        {
          //cout << "Could not munmap." << endl;
          perror("munmap");
        }
      }
    }
  }
  //cout << "~BitstreamManager () done." << endl;
}


