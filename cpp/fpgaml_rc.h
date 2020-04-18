// Copyright QUB 2018

#ifndef fpgaml_rc_h
#define fpgaml_rc_h

namespace FPGAML
{

enum class RC
{
  ML_SUCCESS,
  ML_ERROR,
  ML_INIT_ERROR,
  ML_WRONG_PARAM,
  ML_ROUTE_ERROR,
  ML_HMAP_INSERT_ERROR,
  ML_HMAP_NOT_FOUND_ERROR,
  ML_ERROR_TOO_MANY_NDR,
  ML_ERROR_CMDQ_RELEASED
};

} // namespace FPGAML

#endif
