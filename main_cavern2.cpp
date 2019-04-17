#include <QCoreApplication>
#include <QFile>
#include <QXmlStreamWriter>
#include <QXmlStreamAttributes>

#include <stdio.h>


int main(int argc, char *argv[])
{
  // QCoreApplication a(argc, argv);
  //  return a.exec();

    //read part, find different part
    class hex8{
    public:
      long  id;
      long type=12;
      long materialid;
      long offset=8;
      long nlist[8];
    };

    class tretra{
    public:
      long  id;
      long type=10;
      long materialid;
      long offset =4;
      long nlist[4];
    };

    class pyra{
    public:
      long  id;
      long type=14;
      long materialid;
      long offset =5;
      long nlist[5];
    };

    class nodep{
    public:
        long id;
        double x;
        double y;
        double z;
    };

    hex8 hextemp;
    tretra tretemp;
    pyra pyratemp;

    nodep nodetemp;

    unsigned long cavernElementSize = 29854;


    QFile elementfile("/home/ning/test/model_cavern/NN5/data/NN5_ELEM.dat");
    elementfile.open(QFile::ReadOnly);
    std::vector<hex8> elemvector;

    unsigned long linenum = 0;
    while (!elementfile.atEnd())
    {
        QString line = elementfile.readLine();

        QStringList linedata = line.split(',');

        hextemp.id = linedata[0].toLong();

        if((linenum >= 0) && (linenum< cavernElementSize))
            hextemp.materialid = 1;
        else
            hextemp.materialid = 0;

        for(int i =1; i<linedata.size(); i++ )
        {
            hextemp.nlist[i-1] = linedata[i].toLong()-1;


        }
        elemvector.push_back(hextemp);

        linenum++;

    }
    elementfile.close();

    QFile nodefile("/home/ning/test/model_cavern/NN5/data/NN5_NODE.dat");
    nodefile.open(QFile::ReadOnly);
    std::vector<nodep> nodevector;
    while (!nodefile.atEnd())
    {
        QString line = nodefile.readLine();

        QStringList linedata = line.split(',');

        nodetemp.id = linedata[0].toLong();
        nodetemp.x = linedata[1].toDouble();
        nodetemp.y = linedata[2].toDouble();
        nodetemp.z = linedata[3].toDouble();

        nodevector.push_back(nodetemp);

    }
    nodefile.close();



    unsigned long totalenum =0;

    totalenum = elemvector.size()+ totalenum;
    printf("v1: %ld \n", totalenum);

    QFile partitionfile("/home/ning/test/model_cavern/NN5/data/model_remove.mesh.epart.40");
    partitionfile.open(QFile::ReadOnly);
    std::vector<long> partitionvector;
    while (!partitionfile.atEnd())
    {
        QString line = partitionfile.readLine();

        partitionvector.push_back(line.toLong()+1);

    }
    partitionfile.close();


    unsigned long NodeNum = nodevector.size();
    unsigned long EleNum = elemvector.size()-cavernElementSize;

    unsigned long i ;

    QString stemp;
    char* sline = new char[512];


    QXmlStreamWriter *xmlstream = new QXmlStreamWriter();
    //set auto format
    xmlstream->setAutoFormatting(1);

    QFile modelfile("/home/ning/test/model_cavern/NN5/model.vtu");
    modelfile.open(QIODevice::WriteOnly);
    xmlstream->setDevice(&modelfile);
    xmlstream->writeStartDocument();
    // Begin writing

//    xmlstream->writeComment("shuju");

    xmlstream->writeStartElement("VTKFile");
    QXmlStreamAttributes attnode;
    attnode.append("type", "UnstructuredGrid");
    attnode.append("version","0.1");
    attnode.append("byte_order","LittleEndian");
    //attnode.append("compressor", "vtkZlibDataCompressor");

    xmlstream->writeAttributes(attnode);
    attnode.clear();


        xmlstream->writeStartElement("UnstructuredGrid");
            xmlstream->writeStartElement("Piece");
                stemp.sprintf("%ld", NodeNum);
                attnode.append("NumberOfPoints", stemp);
                stemp.sprintf("%ld", EleNum);
                attnode.append("NumberOfCells",QStringLiteral("%1").arg(EleNum));

                xmlstream->writeAttributes(attnode);   //Write number of point and cells
                attnode.clear();
                    //In Each Piece Two Kind of DATA must be Included: 1. Points 2. Cells. and Two Kind of DATA could be Included: 1. PointData 2. CellData
                    //Here write empty PointData
                    xmlstream->writeStartElement("PointData");
                    xmlstream->writeCharacters(" ");
                    xmlstream->writeEndElement();

                    //Here write Material ID as the CellData
                    xmlstream->writeStartElement("CellData");
                        xmlstream->writeStartElement("DataArray");
                        attnode.append("type", "UInt32");
                        attnode.append("Name","MaterialIDs");
                        attnode.append("format","ascii");

                        xmlstream->writeAttributes(attnode);   //Write number of point and cells

                        attnode.clear();

                        xmlstream->writeCharacters("\n");

                        for(i = cavernElementSize; i < elemvector.size(); i++)
                        {
                            //stemp.sprintf("%ld \n", elemvector[i].materialid);
                            stemp.sprintf("%ld \n", partitionvector[i-cavernElementSize]);
                            //printf("%s",stemp.data());
                            xmlstream->writeCharacters(stemp);
                        }
/*
                        for(i = 0; i < cavernElementSize; i++)
                        {
                            stemp.sprintf("%ld \n", elemvector[i].materialid);
                            //printf("%s",stemp.data());
                            xmlstream->writeCharacters(stemp);
                        }
*/

                        xmlstream->writeCharacters(" ");

                        xmlstream->writeEndElement();
                    xmlstream->writeEndElement();

                    //Here Write Points
                    //Points coors in the order x1,y1,z1,x2,y2,z2......
                    xmlstream->writeStartElement("Points");
                        xmlstream->writeStartElement("DataArray");
                        attnode.append("type", "Float64");
                        attnode.append("Name","Points");
                        attnode.append("NumberOfComponents", "3");
                        attnode.append("format","ascii");

                        xmlstream->writeAttributes(attnode);   //Write number of point and cells

                        attnode.clear();

                        xmlstream->writeCharacters("\n");
                        for(i = 0; i< nodevector.size(); i++)
                        {
                            stemp.sprintf("%lf %lf %lf\n", nodevector[i].x, nodevector[i].y, nodevector[i].z);
                            xmlstream->writeCharacters(stemp);
                        }

                        nodefile.close();
                        xmlstream->writeCharacters(" ");

                        xmlstream->writeEndElement();
                    xmlstream->writeEndElement();

                    //Here Write Cells
                    //Cells Must Include:1. Connectivities 2. Offset(each cell has how many nodes) 3. Types
                    xmlstream->writeStartElement("Cells");
                        //Connectivities array
                        xmlstream->writeStartElement("DataArray");
                        attnode.append("type", "Int64");
                        attnode.append("Name","connectivity");
                        attnode.append("format","ascii");

                        xmlstream->writeAttributes(attnode);   //Write number of point and cells

                        attnode.clear();

                        xmlstream->writeCharacters("\n");


                        /////
                        for(i = cavernElementSize; i < elemvector.size(); i++)
                        {
                            stemp.sprintf("%ld %ld %ld %ld %ld %ld %ld %ld\n", elemvector[i].nlist[0],elemvector[i].nlist[1],elemvector[i].nlist[2],elemvector[i].nlist[3],elemvector[i].nlist[4],elemvector[i].nlist[5],elemvector[i].nlist[6],elemvector[i].nlist[7]);
                            //printf("%s",stemp.data());
                            xmlstream->writeCharacters(stemp);
                        }

/*
                        for(i = 0; i < cavernElementSize; i++)
                        {
                            stemp.sprintf("%ld %ld %ld %ld %ld %ld %ld %ld\n", elemvector[i].nlist[0],elemvector[i].nlist[1],elemvector[i].nlist[2],elemvector[i].nlist[3],elemvector[i].nlist[4],elemvector[i].nlist[5],elemvector[i].nlist[6],elemvector[i].nlist[7]);
                            //printf("%s",stemp.data());
                            xmlstream->writeCharacters(stemp);
                        }
*/
                         /////
                        xmlstream->writeCharacters(" ");

                        xmlstream->writeEndElement();
                        //Offset Array
                        xmlstream->writeStartElement("DataArray");
                        attnode.append("type", "Int64");
                        attnode.append("Name","offsets");
                        attnode.append("format","ascii");

                        xmlstream->writeAttributes(attnode);   //Write number of point and cells

                        attnode.clear();

                        xmlstream->writeCharacters(" ");
                        long totaloffset =0;

                        for(i = cavernElementSize; i < elemvector.size(); i++)
                        {
                            totaloffset = elemvector[i].offset+totaloffset;
                            stemp.sprintf("%ld\n", totaloffset);
                            //printf("%s",stemp.data());
                            xmlstream->writeCharacters(stemp);
                        }
/*
                        for(i = 0; i < cavernElementSize; i++)
                        {
                            totaloffset = elemvector[i].offset+totaloffset;
                           stemp.sprintf("%ld\n", totaloffset);
                            //printf("%s",stemp.data());
                            xmlstream->writeCharacters(stemp);
                        }
*/
                         xmlstream->writeCharacters(" ");

                        xmlstream->writeEndElement();

                        //Types array
                        xmlstream->writeStartElement("DataArray");
                        attnode.append("type", "UInt8");
                        attnode.append("Name","types");
                        attnode.append("format","ascii");

                        xmlstream->writeAttributes(attnode);   //Write number of point and cells

                        attnode.clear();

                        xmlstream->writeCharacters("\n");
                        for(i = cavernElementSize; i < elemvector.size(); i++)
                        {
                            stemp.sprintf("%ld\n", elemvector[i].type);
                            //printf("%s",stemp.data());
                            xmlstream->writeCharacters(stemp);
                        }
/*
                        for(i = 0; i < cavernElementSize; i++)
                        {
                           stemp.sprintf("%ld\n", elemvector[i].type);
                            //printf("%s",stemp.data());
                            xmlstream->writeCharacters(stemp);
                        }
*/

                        xmlstream->writeCharacters(" ");

                        xmlstream->writeEndElement();

                    xmlstream->writeEndElement();

                    //End Write Cells

            xmlstream->writeEndElement();
        xmlstream->writeEndElement();

    xmlstream->writeEndElement();

//End Writing
    xmlstream->writeEndDocument();

    modelfile.close();



    return 0;

}
