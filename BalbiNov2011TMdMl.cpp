/*

Copyright (C) 2012 ForeFire Team, SPE, Universit� de Corse.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 US

*/

#include "BalbiNov2011TMdMl.h"

namespace libforefire {

/* name of the model */
const string BalbiNov2011TMdMl::name = "BalbiNov2011TMdMl";

/* instantiation */
PropagationModel* getBalbiNov2011TMdMlModel(const int & mindex, DataBroker* db) {
	return new BalbiNov2011TMdMl(mindex, db);
}

/* registration */
int BalbiNov2011TMdMl::isInitialized =
        FireDomain::registerPropagationModelInstantiator(name, getBalbiNov2011TMdMlModel );

/* constructor */
BalbiNov2011TMdMl::BalbiNov2011TMdMl(const int & mindex, DataBroker* db)
: PropagationModel(mindex, db) {

	/* defining the properties needed for the model */
	slope = registerProperty("slope");
	normalWind = registerProperty("normalWind");
	Rhod = registerProperty("fuel.Rhod");
	Rhol = registerProperty("fuel.Rhol");
	Md = registerProperty("fuel.Md");
	Ml = registerProperty("fuel.Ml");
	sd = registerProperty("fuel.sd");
	sl = registerProperty("fuel.sl");
	e = registerProperty("fuel.e");
	Sigmad = registerProperty("fuel.Sigmad");
	Sigmal = registerProperty("fuel.Sigmal");
	stoch = registerProperty("fuel.stoch");
	RhoA = registerProperty("fuel.RhoA");
	Ta = registerProperty("fuel.Ta");
	Tau0 = registerProperty("fuel.Tau0");
	Deltah = registerProperty("fuel.Deltah");
	DeltaH = registerProperty("fuel.DeltaH");
	Cp = registerProperty("fuel.Cp");
	Ti = registerProperty("fuel.Ti");
	X0 = registerProperty("fuel.X0");
	r00 = registerProperty("fuel.r00");
	Blai = registerProperty("fuel.Blai");
	temperature = registerProperty("temperature");
	liveMoisture = registerProperty("moisture");
	deadMoisture = Md; //registerProperty("deadMoisture");

	/* allocating the vector for the values of these properties */
	if ( numProperties > 0 ) properties =  new double[numProperties];

	/* registering the model in the data broker */
	dataBroker->registerPropagationModel(this);

	/* Definition of the coefficients */
	cooling = 0.;
	if ( params->isValued("BalbiNov2011TMdMl.cooling") )
		cooling = params->getDouble("BalbiNov2011TMdMl.cooling");
	Cpa = 1004.;
	if ( params->isValued("BalbiNov2011TMdMl.Cpa") )
		Cpa = params->getDouble("BalbiNov2011TMdMl.Cpa");
	adjustementSlope = 1.;
	if ( params->isValued("BalbiNov2011TMdMl.adjustementSlope") )
		adjustementSlope = params->getDouble("BalbiNov2011TMdMl.adjustementSlope");
	adjustementWind = 1.;
	if ( params->isValued("BalbiNov2011TMdMl.adjustementWind") )
		adjustementWind = params->getDouble("BalbiNov2011TMdMl.adjustementWind");
	if (adjustementWind != 1 && adjustementSlope != 1){
		cout << "Warning, model Balbi Nov 2011 running wih adjustements Slope "<<adjustementSlope<<" Wind "<<adjustementWind<< endl;
	}
}

/* destructor (shoudn't be modified) */
BalbiNov2011TMdMl::~BalbiNov2011TMdMl() {
}

/* accessor to the name of the model */
string BalbiNov2011TMdMl::getName(){
	return name;
}

/* *********************************************** */
/* Model for the propagation velovity of the front */
/* *********************************************** */

double BalbiNov2011TMdMl::getSpeed(double* valueOf){

	double lRhod = valueOf[Rhod];
	double lRhol = valueOf[Rhol];
	double lMd  = valueOf[deadMoisture];
	double lMl  = valueOf[liveMoisture];
	double lsd  = valueOf[sd];
	double lsl  = valueOf[sl];
	double le   = valueOf[e];
	double lSigmad = valueOf[Sigmad];
	double lSigmal = valueOf[Sigmal];
	double lstoch = valueOf[stoch];
	double lRhoA  = valueOf[RhoA];
	double lTa  = valueOf[temperature];
	double lTau0  = valueOf[Tau0];
	double lDeltah   = valueOf[Deltah];
	double lDeltaH = valueOf[DeltaH];
	double lCp = valueOf[Cp];
	double lTi  = valueOf[Ti];
	double lX0  = valueOf[X0];
	double lr00  = valueOf[r00];
	double lai = valueOf[Blai];

	double cosCurv = 1;
	
	if(le <= 0 ) return 0;
	
	//cout << "mm = " << lMl << endl;

	double Betad =   lSigmad /(le* lRhod);
	double Betal =   lSigmal /(le* lRhol);
        // '--> Ok if Rhod & Rhol are particle densities and if Sigmad,
        //      Sigmal & e are "bulk" properties.
        // However, that does not seem consistent with Rothermel.cpp...
	double Sd = lsd * le * Betad;
	double Sl = lsl * le * Betal;
	double nu = min((Sd) / lai, 1.);
	double normal_wind = adjustementWind*valueOf[normalWind];
	/* double B = 5.6E-8; */
        double B = 5.670373E-8;
	double a = lDeltah/ ((lCp*(lTi-lTa)));
	double r0 = lsd * lr00;
	double A0 = (lX0*lDeltaH)/(4*lCp*(lTi-lTa));
	/* double xsi = ((lMl-lMd)*((lSigmal/lSigmad)*(lDeltah/lDeltaH))); */
        double xsi = ((lMl-lMd)*((Sd/Sl)*(lDeltah/lDeltaH))); // cf. Santoni et al., 2011
	double A  = cosCurv * (nu*A0 / (1 + a * lMd)) * (1-xsi);
	double T = lTa + ( lDeltaH*(1-lX0)*(1-xsi) )     / ((lstoch+1)*Cpa);
	double R00 = (B*T*T*T*T)   / (lCp*(lTi-lTa));
	double R = 0;
	double R0 =0;
	double u00 = (2*lai*(lstoch+1)*T*lRhod)/(lRhoA*lTa*lTau0);
	double u0 = nu * u00;

	double  tanGamma =  adjustementSlope*valueOf[slope] + (normal_wind/u0);

	double gamma = atan(tanGamma);

	R0 = (le / lSigmad)   * (R00) / (1 + a * lMd) * Sd/(Sd+Sl) * Sd/(Sd+Sl);
	if ( gamma > 0. ) {
          // cf. Balbi et al., 2009 -> (9) => (11a) & (11b)
          /* double geomFactor = r0*((1+sin(gamma)-cos(gamma))/(1.+cos(gamma))); */
          double geomFactor = r0/cos(gamma)*(1 + sin(gamma) - cos(gamma));
          /* double Rt = R0 + A*geomFactor ; */
          double Rt = R0 + A*geomFactor - r0/cos(gamma);
          R = 0.5*( Rt + sqrt( Rt*Rt + 4.*r0*R0/cos(gamma) ) );


	} else {
		R = R0;
	}
	//COUT << "MM = " << LTA << ENDL;
	return R ;

}

}
