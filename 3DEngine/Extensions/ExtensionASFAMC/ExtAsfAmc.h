/*
 * ExtAsfAmc.h
 *
 *  Created on: Jan 4, 2014
 *      Author: yama
 */

#ifndef EXTASFAMC_H_
#define EXTASFAMC_H_

#include "egExtensions.h"

//#include "egPrerequisites.h"
//#include "egExtensions.h"

namespace Horde3DAsfAmc {

using namespace Horde3D;

class ExtAsfAmc: public Horde3D::IExtension {
public:
	ExtAsfAmc();
	virtual ~ExtAsfAmc();

	virtual const char *getName();
	virtual bool init();
	virtual void release();

};

} /* namespace Horde3DAsfAmc */

#endif /* EXTASFAMC_H_ */
