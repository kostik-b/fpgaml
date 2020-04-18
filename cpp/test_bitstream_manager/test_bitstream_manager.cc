// Copyright QUB 2018

#include <stdio.h>
#include <bitstream_manager.h>
#include <iostream>
#include <string>

using namespace std;
int main (int argc, char** argv)
{
  if (argc < 2)
  {
    printf ("Error - please supply path to xml file\n");
    return 1;
  }

  setenv ("FPGAML_XML_PATH", argv[1], 1);

  FPGAML::BitstreamManager bs_mng;

  string xml_file = "incr-3d_1.xml";
  string target;
  bs_mng.get_target (xml_file, target);
  cout << "target : " << target << endl ;

  string rs;
  bs_mng.get_resource_string(xml_file, rs);
  cout << "resource string : " << rs << endl;

  vector<string> offsets;
  bs_mng.get_offset_address (xml_file, offsets);
  cout << "n_args : " << offsets.size() << endl;

  for(unsigned i=0; i < offsets.size(); i++)
     cout << string("offset_address_") + to_string(i) << " " << offsets[i] << " = " << strtoul(offsets[i].c_str(), 0, 16) << endl;

  unsigned length;
  bs_mng.get_bs_length (xml_file, length);
  cout << "length : " << length << endl;

  string bitstream;
  bs_mng.get_bit_stream (xml_file, bitstream);
  cout << "bitstream : " << bitstream.substr (0, 100) << endl;


  // done
  return 0;
}
