
#include        "compressed_io.hpp"
#include        "Parallel.hpp"
#include        "Control2.hpp"
#include        "Ion.hpp"

/*****************************************************
 *                                                   *
 *            psp_formatter_read_header              *
 *                                                   *
 *****************************************************/

/* This function returns the header data of a vpp file.  

   Entry - myparall
           fname - name of vpp file
   Exit -  header data: comment, psp_type, version, nfft, unita, atom, amass, zv

   Returns true if fname exists otherwise false.
*/

bool psp_formatter_read_header(Parallel *myparall, char *fname, 
                              char *comment, int *psp_type, int *version, int nfft[], double unita[], 
                              char *atom, double *amass, double *zv) 
{
   int i,ifound;

   if (myparall->is_master())
   {
      ifound = cfileexists(fname);
   }
   myparall->Brdcst_iValue(0,0,&ifound);

   if (ifound>0) 
   {
      if (myparall->is_master())
      {
          openfile(5,fname,"r");
          cread(5,comment,80);
          comment[79] = '\0';
          i = 78;
          while (comment[i] == ' ')
            comment[i--] = '\0';

          iread(5,psp_type,1);
          iread(5,version,1);
          iread(5,nfft,3);
          dread(5,unita,9);
          cread(5,atom,2);
          dread(5,amass,1);
          dread(5,zv,1);
          closefile(5);
       }
       myparall->Brdcst_cValues(0,0,80,comment);
       myparall->Brdcst_iValue(0,0,psp_type);
       myparall->Brdcst_iValue(0,0,version);
       myparall->Brdcst_iValues(0,0,3,nfft);
       myparall->Brdcst_Values(0,0,9,unita);
       myparall->Brdcst_cValues(0,0,2,atom);
       myparall->Brdcst_Values(0,0,1,amass);
       myparall->Brdcst_Values(0,0,1,zv);
   }

   return (ifound>0);
}



/*****************************************************
 *                                                   *
 *                psp_formatter_check                *
 *                                                   *
 *****************************************************/

void psp_formatter_check(Parallel *myparall, Ion *myion, Control2& control)
{
   char fname[256],comment[80],atom[2];
   int psp_type,version,nfft[3];
   double unita[9],amass,zv;
   bool reformat;
   double tol=1.0e-9;


   for (auto ia=0; ia<myion->nkatm; ++ia)
   {
      strcpy(fname,myion->atom(ia));
      strcat(fname,".vpp");
      control.add_permanent_dir(fname);

      reformat = true;
      if (psp_formatter_read_header(myparall,fname,
                            comment, &psp_type, &version, nfft, unita,
                            atom, &amass, &zv))
      {
         myion->set_zv_psp(ia,zv);

         reformat = false;
         for (auto i=0; i<9; ++i)
            reformat = reformat || (fabs(control.unita1d(i)-unita[i])>tol);
         for (auto i=0; i<3; ++i)
            reformat = reformat || (control.ngrid(i)!=nfft[i]);
         //reformat = reformat || (control.pversion!=version);
      }

      if (reformat)
      {
         printf("Need to reformat %s\n", fname);
      }

   }
   /* set the total ion charge in control which in turn sets ispin and ne */
   control.set_total_ion_charge(myion->total_zv());

}