// FaceDetectionDoc.cpp : implementation of the CFaceDetectionDoc class
//

#include "stdafx.h"
#include "FaceDetection.h"

#include "FaceDetectionDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFaceDetectionDoc

IMPLEMENT_DYNCREATE(CFaceDetectionDoc, CDocument)

BEGIN_MESSAGE_MAP(CFaceDetectionDoc, CDocument)
	//{{AFX_MSG_MAP(CFaceDetectionDoc)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFaceDetectionDoc construction/destruction

CFaceDetectionDoc::CFaceDetectionDoc()
{
	// TODO: add one-time construction code here
	int i,j;
	m_hDIB=NULL;
	for( i=0; i<ImgRange; i++)
		for ( j=0; j<ImgRange; j++)
		{
			emymapc[i][j] = false;
			emymapl[i][j] = false;
			//lab[i][j] = false;
		}
	for( i=0;i<ImgRange; i++)
		for ( j=0; j<ImgRange; j++)
			lab[i][j] = false;

}

CFaceDetectionDoc::~CFaceDetectionDoc()
{
}

BOOL CFaceDetectionDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CFaceDetectionDoc serialization

void CFaceDetectionDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFaceDetectionDoc diagnostics

#ifdef _DEBUG
void CFaceDetectionDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFaceDetectionDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFaceDetectionDoc commands

BOOL CFaceDetectionDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	CFile fileM;
	CFileException feM;
	m_hDIB = NULL;
	if (!fileM.Open(lpszPathName, CFile::modeRead | CFile::shareDenyWrite, &feM))
	{
		// ʧ��
		ReportSaveLoadException(lpszPathName, &feM,FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);	
		// ����FALSE
		return FALSE;
	}
	if (!fileM2.Open(lpszPathName, CFile::modeRead | CFile::shareDenyWrite, &feM))
	{
		// ʧ��
		ReportSaveLoadException(lpszPathName, &feM,FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);	
		// ����FALSE
		return FALSE;
	}
		TRY
	{
		m_hDIB = ::ReadDIBFile(fileM);
	}
	CATCH (CFileException, eLoad)
	{
		// ��ȡʧ��
		fileM.Abort();		
		// �ָ������״
		EndWaitCursor();		
		// ����ʧ��
		ReportSaveLoadException(lpszPathName, eLoad,
			FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);	
		// ����DIBΪ��
		m_hDIB = NULL;	
		fileM.Close();
		// ����FALSE
		return FALSE;
	}
	END_CATCH
	m_hDIBtemp=(HDIB)CopyHandle(m_hDIB);
	//if(fileM.Read(&bmfHeaderMulspec,sizeof(BITMAPFILEHEADER)) != sizeof(BITMAPFILEHEADER))
	//return FALSE;

	fileM.Close();
	// TODO: Add your specialized creation code here
	return TRUE;
}
bool CFaceDetectionDoc::LightingCompensate()
{

	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	int width,height;
	LPBYTE lpData;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	//����ϵ��
	const float thresholdco = 0.05;
	//���ظ������ٽ糣��
	const int thresholdnum = 100;
	//�Ҷȼ�����
	int histogram[256];
	for(int i =0;i<256;i++)
		histogram[i] = 0;
	//���ڹ���С��ͼƬ���ж�
	if(width*height*thresholdco < thresholdnum)
		return false;
	int colorr,colorg,colorb;
	long lOffset;
	//��������ͼƬ
	for( i=0;i<height;i++)
		for(int j=0;j<width;j++)
		{	
			//�õ��������ݵ�ƫ��
			lOffset = i*wBytesPerLine + j*3;
			//�õ�rgbֵ
			colorb = *(lpData+lOffset++);
			colorg = *(lpData+lOffset++);
			colorr = *(lpData+lOffset++);
			//����Ҷ�ֵ
			int gray = (colorr * 299 + colorg * 587 + colorb * 114)/1000;
			histogram[gray]++;
		}
		int calnum =0;
		int total = width*height;
		int num;
		//�����ѭ���õ�����ϵ��thresholdco���ٽ�Ҷȼ�
		for(i =0;i<256;i++)
		{
			if((float)calnum/total < thresholdco)
			{
				calnum+= histogram[255-i];
				num = i;
			}
			else
				break;
		}
		int averagegray = 0;
		calnum =0;
		//�õ����������������ܵĻҶ�ֵ
		for(i = 255;i>=255-num;i--)
		{
			averagegray += histogram[i]*i;
			calnum += histogram[i];
		}
		averagegray /=calnum;
		//�õ����߲�����ϵ��
		float co = 255.0/(float)averagegray;
		//�����ѭ����ͼ����й��߲���
		for(i =0;i<height;i++)
			for(int j=0;j<width;j++)
			{	
				//�õ����ݱ���
				lOffset =i*wBytesPerLine + j*3;
				//�õ���ɫ����
				colorb = *(lpData+lOffset);
				//����
				colorb *=co;
				//�ٽ��ж�
				if(colorb >255)
					colorb = 255;
				//����
				*(lpData+lOffset) = colorb;
				//��ɫ����
				colorb = *(lpData+lOffset+1);
				colorb *=co;
				if(colorb >255)
					colorb = 255;
				*(lpData+lOffset+1) = colorb;
				//��ɫ����
				colorb = *(lpData+lOffset+2);
				colorb *=co;
				if(colorb >255)
					colorb = 255;
				*(lpData+lOffset+2) = colorb;
			}
	return TRUE;
}
void CFaceDetectionDoc::RgbtoYcb(HDIB hDIB,LPBYTE lpYcb)
{
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL)hDIB);
	int width,height;
	WORD wBytesPerLine;
	LPBYTE lpData;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	//�õ�ͼ��Ļ�����Ϣ	
	wBytesPerLine = lLineBytesMulspec;//�õ�ͼƬÿ�е�������ռ�ֽڸ���

	long lOffset;
	//�����ѭ��ʵ�ִ�rgb��ycc��ת��
	for(int i=0;i<height;i++)
		for(int j=0;j<width;j++)
		{
			
			lOffset = i*wBytesPerLine + j*3;	//�õ��������ݵ�ƫ��
			//�õ�rgb��ֵ
			int b = *(lpData + lOffset);
			int g = *(lpData + lOffset+1);
			int r = *(lpData + lOffset+2);
			//����õ�y��cr��cb����ֵ
			int Y = (257*r+504*g+98*b)/1000+16;
			int Cr = (439*r-368*g-71*b)/1000+128;
			int Cb = (-148*r-291*g+439*b)/1000+128;
			//�������õ�����ֵ
			*(lpYcb+lOffset++) = Y;
			*(lpYcb+lOffset++) = Cr;
			*(lpYcb+lOffset++) = Cb;
		}
	::GlobalUnlock((HGLOBAL) hDIB);
}

//////////////*Ƥ����ɫ��ģ*//////////////
int CFaceDetectionDoc::_Cb(int Y)
{	
	int Cb;
	//������Ⱥ�С�����
	if(Y<Kl)
		Cb = 108 + ((Kl-Y)*10)/(Kl-Ymin);
	//���Ⱥܴ�����
	else if(Y>Kh)
		Cb = 108 + ((Y-Kh)*10)/(Ymax - Kh);
	else 
		Cb = -1;
	return Cb;
}

int CFaceDetectionDoc::_Cr(int Y)
{
	int Cr;
	//���Ⱥ�С�����
	if(Y<Kl)
		Cr = 154 - ((Kl-Y)*10)/(Kl-Ymin);
	//���Ⱥܴ�����
	else if(Y>Kh)
		Cr = 154 - ((Y-Kh)*22)/(Ymax - Kh);
	else
		Cr = -1;
	return Cr;
}
int CFaceDetectionDoc::_WCr(int Y)
{
	int WCr;
	if(Y<Kl)
		//���Ⱥ�С�����
		WCr = WLcr + ((Y-Ymin)*(Wcr-WLcr))/(Kl-Ymin);
	else if(Y>Kh)
		//���Ⱥܴ�����
		WCr = WHcr + ((Ymax-Y)*(Wcr-WHcr))/(Ymax-Kh);
	else WCr = -1;
	return WCr;
}

int CFaceDetectionDoc:: _WCb(int Y)
{
	int WCb;
	if(Y<Kl)
		//���Ⱥ�С�����
		WCb = WLcb + ((Y-Ymin)*(Wcb-WLcb))/(Kl-Ymin);
	else if(Y>Kh)
		//���Ⱥܴ�����
		WCb = WHcb + ((Ymax-Y)*(Wcb-WHcb))/(Ymax-Kh);
	else WCb = -1;
	return WCb;
}
void CFaceDetectionDoc::YccTransform(LPBYTE lpYcc,WORD wBytesPerLine,int height,int width)
{	
	int Y,Cr,Cb;
	long lOffset;
	//�����ѭ��ʵ��yccɫ�ʿռ�ķ�����ת��
	for(int i=0;i<height;i++)
		for(int j=0;j<width;j++)
		{	
			//�õ�����ƫ��
			lOffset = i*wBytesPerLine + j*3;
			//�õ�y��Cr��Cb��ֵ
			Y = *(lpYcc+lOffset);
			Cr = *(lpYcc+lOffset+1);
			Cb = *(lpYcc+lOffset+2);
			//���y��ֵ�������ٽ�ֵ֮�䣬���ֲ���
			if(Y>=Kl && Y<=Kh)
				continue;
			//���÷�����ת����������Cr��Cb����ֵ
			Cr = (Cr-_Cr(Y))*(Wcr/_WCr(Y))+_Cr(Kh);
			Cb = (Cb-_Cb(Y))*(Wcb/_WCb(Y))+_Cb(Kh);
			*(lpYcc+lOffset+1) = Cr;
			*(lpYcc+lOffset+2) = Cb;
		}
}

void CFaceDetectionDoc::faceear(LPBYTE lpYcc, WORD wBytesPerLine, int height,int width, bool flag[ImgRange][ImgRange])
{	
	//��ʼ����־λ
	for (int i=0; i<ImgRange; i++)
		for (int j=0; j<ImgRange; j++)
		{
			flag[i][j] = false;
		}
	long lOffset;
	int Cr;
	int Cb;
	for (i=0; i<height; i++)
		for (int j=0; j<width; j++)
		{	
			//�õ�ƫ��
			lOffset = i*wBytesPerLine + j*3;
			//�õ�Cr��Cb��ֵ
			Cr = *(lpYcc+lOffset+1);
			Cb = *(lpYcc+lOffset+2);
			//������ɫ��ģ
			if(FaceModeling(Cr,Cb))
			{	
				//�޸ı�־λ
				flag[i][j] = true;
			}
		}	
}
BOOL CFaceDetectionDoc::FaceModeling(int Cr,int Cb)
{	
	//Cb��ϵ������
	const float cx = 122.453;//114.38;
	//cr��ϵ������
	const float cy = 158.442;//160.02;
	//�Ƕȳ���
	const float theta = 2.53;
	//x���ߺ�y���ߵ���������
	const float ecx = 1.60;
	const float ecy = 2.41;
	//����
	const float a = 25.39;
	//����
	const float b = 14.03;
	//���ƶȳ���
	const float judge = 0.5;
	//����õ�x����ֵ
	float  x = cos(theta)*(Cb-cx)+sin(theta)*(Cr-cy);
	//y����ֵ
	float  y = -sin(theta)*(Cb -cx)+cos(theta)*(Cr-cy);
	//����������
	float temp = pow(x-ecx,2)/pow(a,2)+pow(y-ecy,2)/pow(b,2);
	//�������Ҫ�󷵻��棬�����
	if(fabs(temp-1.0)<judge)
		return TRUE;
	else
		return FALSE;
}

void CFaceDetectionDoc::Skintone() 
{	
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	int width,height;
	LPBYTE lpData;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	lpYcc = new BYTE[wBytesPerLine * height];
	RgbtoYcb(m_hDIB,lpYcc);
	YccTransform(lpYcc, wBytesPerLine,height,width);
	faceear(lpYcc,wBytesPerLine,height,width,flag);	
	lpData =(LPBYTE)::GlobalLock(lpDIB)+sizeof(BITMAPINFOHEADER);	
	for (int i=0; i<height; i++)
		for (int j=0; j<width; j++)
		{	
			long lOffset = i*wBytesPerLine + j*3;
			if (flag[i][j] == true)
			{
				*(lpData + lOffset++) = 255;
				*(lpData + lOffset++) = 255;
				*(lpData + lOffset++) = 255;
			}
			else
			{
				*(lpData + lOffset++) = 0;
				*(lpData + lOffset++) = 0;
				*(lpData + lOffset++) = 0;
			}
		}
		
		::GlobalUnlock(lpDIB);
}

void CFaceDetectionDoc::Dilation()
{
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	int width,height;
	LPBYTE lpData;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	//����һ�����������С��ͬ���ڴ�
	LPBYTE lpTemp;
	lpTemp = (LPBYTE) new BYTE[wBytesPerLine * height];	
	long lOffsetJudge;
	long lOffset;
	for (int i=1; i<height-1; i++)
		for (int j=1; j<width-1; j++)
		{	
			lOffset = i*wBytesPerLine + j*3;
			//�����ǰ��Ϊ��ɫ������ѭ��
			if(*(lpData + lOffset) == 255)
			{
				*(lpTemp + lOffset++) = 255;
				*(lpTemp + lOffset++) = 255;
				*(lpTemp + lOffset++) = 255;
				continue;
			}
			//���򿼲����������ĸ���
			else
			{	
				lOffsetJudge = (i-1)*wBytesPerLine + j*3;
				//�������ĵ�Ϊ��ɫ
				if(*(lpData + lOffsetJudge) == 255)
				{	//����Ϊ��ɫ��������ѭ��
					*(lpTemp + lOffset++) = 255;
					*(lpTemp + lOffset++) = 255;
					*(lpTemp + lOffset++) = 255;
					continue;
				}

				//��������ĵ�
				lOffsetJudge = (i+1)*wBytesPerLine + j*3;
				if(*(lpData + lOffsetJudge) == 255)
				{
					*(lpTemp + lOffset++) = 255;
					*(lpTemp + lOffset++) = 255;
					*(lpTemp + lOffset++) = 255;
					continue;
				}
				
				//������ߵĵ�
				lOffsetJudge = i*wBytesPerLine + (j-1)*3;
				if(*(lpData + lOffsetJudge) == 255)
				{
					*(lpTemp + lOffset++) = 255;
					*(lpTemp + lOffset++) = 255;
					*(lpTemp + lOffset++) = 255;
					continue;
				}
				//�����ұߵĵ�
				lOffsetJudge = i*wBytesPerLine + (j+1)*3;
				if(*(lpData + lOffsetJudge) == 255)
				{
					*(lpTemp + lOffset++) = 255;
					*(lpTemp + lOffset++) = 255;
					*(lpTemp + lOffset++) = 255;
					continue;
				}
				//����������Ҷ��Ǻ�ɫ�㣬�����ʱ����ĵ�����Ϊ��ɫ
				lOffset = i*wBytesPerLine + j*3;
				*(lpTemp + lOffset++) = 0;
				*(lpTemp + lOffset++) = 0;
				*(lpTemp + lOffset++) = 0;

			}
				
		}
		//����ͼ�����ܵĵ㣬����Ϊ��ɫ
		for(i=0; i<height; i++)
		{
			lOffset = i*wBytesPerLine;
			{
				*(lpTemp + lOffset++) = 0;
				*(lpTemp + lOffset++) = 0;
				*(lpTemp + lOffset++) = 0;
			}
		}

		for(i=0; i<height; i++)
		{
			lOffset = i*wBytesPerLine + (width-1)*3;
			{
				*(lpTemp + lOffset++) = 0;
				*(lpTemp + lOffset++) = 0;
				*(lpTemp + lOffset++) = 0;
			}
		}

		for(i=0; i<width; i++)
		{
			lOffset = i*3;
			{
				*(lpTemp + lOffset++) = 0;
				*(lpTemp + lOffset++) = 0;
				*(lpTemp + lOffset++) = 0;
			}
		}

		for(i=0; i<width; i++)
		{
			lOffset = (height-1)*wBytesPerLine + i*3;
			{
				*(lpTemp + lOffset++) = 0;
				*(lpTemp + lOffset++) = 0;
				*(lpTemp + lOffset++) = 0;
			}
		}
		//����ʱ����ĵ㿽����ԭ�����������
		memcpy(lpData, lpTemp, wBytesPerLine*height);
		delete [] lpTemp;
		::GlobalUnlock((HGLOBAL) m_hDIB);
}

void CFaceDetectionDoc::Erasion()
{
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	int width,height;
	LPBYTE lpData;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	//����һ�����������С��ͬ���ڴ�
	LPBYTE lpTemp;
	lpTemp = (LPBYTE) new BYTE[wBytesPerLine * height];	
	long lOffsetJudge;
	long lOffset;
	//�����ѭ��ʵ�ָ�ʴ����
	for (int i=1; i<height-1; i++)
		for (int j=1; j<width-1; j++)
		{
			lOffset = i*wBytesPerLine + j*3;
			//���Ϊ��ɫ��
			if (*(lpData+lOffset) == 255)
			{	
				//��������ĵ�
				lOffsetJudge = (i-1)*wBytesPerLine + j*3;
				//����Ǻ�ɫ�Ͱ�ԭ���ĵ�����Ϊ��ɫ��������ѭ��
				if (*(lpData + lOffsetJudge) ==0)
				{
					*(lpTemp + lOffset++) = 0;
					*(lpTemp + lOffset++) = 0;
					*(lpTemp + lOffset++) = 0;
					continue;
				}
				//��������ĵ�
				lOffsetJudge =  (i+1)*wBytesPerLine + j*3;
				if (*(lpData + lOffsetJudge) ==0)
				{
					*(lpTemp + lOffset++) = 0;
					*(lpTemp + lOffset++) = 0;
					*(lpTemp + lOffset++) = 0;
					continue;
				}
				//����ĵ�
				lOffsetJudge = i*wBytesPerLine + (j-1)*3;
				if (*(lpData + lOffsetJudge) ==0)
				{
					*(lpTemp + lOffset++) = 0;
					*(lpTemp + lOffset++) = 0;
					*(lpTemp + lOffset++) = 0;
					continue;
				}
				//����ĵ�
				lOffsetJudge = i*wBytesPerLine + (j+1)*3;
				if (*(lpData + lOffsetJudge) ==0)
				{
					*(lpTemp + lOffset++) = 0;
					*(lpTemp + lOffset++) = 0;
					*(lpTemp + lOffset++) = 0;
					continue;
				}
				//������������ĸ��㶼�ǰ�ɫ��������Ϊ��ɫ
				lOffset = i*wBytesPerLine + j*3;
				*(lpTemp + lOffset)   = 255;
				*(lpTemp + lOffset+1) = 255;
				*(lpTemp + lOffset+2) = 255;
				
			}
			//�����ǰ��Ϊ��ɫ��������ʱ��Ŀ������������Ϊ��ɫ
			else
			{
				*(lpTemp + lOffset)   = 0;
				*(lpTemp + lOffset+1) = 0;
				*(lpTemp + lOffset+2) = 0;
			}
		}
		
		//��ͼ���ܱߵĵ�ȫ������Ϊ��ɫ
		for(i=0; i<height; i++)
		{
			lOffset = i*wBytesPerLine;
				*(lpTemp + lOffset)   = 0;
				*(lpTemp + lOffset+1) = 0;
				*(lpTemp + lOffset+2) = 0;

		}
		
		for(i=0; i<height; i++)
		{
			lOffset = i*wBytesPerLine + (width-1)*3;
				*(lpTemp + lOffset)   = 0;
				*(lpTemp + lOffset+1) = 0;
				*(lpTemp + lOffset+2) = 0;

		}

	for (i=0; i<width; i++)
		{
			lOffset =i*3;
				*(lpTemp + lOffset)   = 0;
				*(lpTemp + lOffset+1) = 0;
				*(lpTemp + lOffset+2) = 0;

		}
	
	for (i=0; i<width; i++)
		{
			lOffset = (height-1)*wBytesPerLine + i*3;
				*(lpTemp + lOffset)   = 0;
				*(lpTemp + lOffset+1) = 0;
				*(lpTemp + lOffset+2) = 0;

		}
	//����ʱ����ĵ㿽����ԭ�����������
	memcpy(lpData, lpTemp, wBytesPerLine*height);
	delete [] lpTemp;
	::GlobalUnlock((HGLOBAL) m_hDIB);
}
void CFaceDetectionDoc::ErasionMouse()
{
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	int width,height;
	LPBYTE lpData;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	//����һ�����������С��ͬ���ڴ�
	LPBYTE lpTemp;
	lpTemp = (LPBYTE) new BYTE[wBytesPerLine * height];	
	long lOffsetJudge;
	long lOffset;
	//����Ĵ���ʵ�ָ�ʴ����
	for (int i=1; i<height-1; i++)
		for (int j=1; j<width-1; j++)
		{	
			//�����ǰ��Ϊ��ɫ
			lOffset = i*wBytesPerLine + j*3;
			if (*(lpData+lOffset) == 255)
			{
				
				//�ж���ߵĵ㣬����Ǻ�ɫ�ľͰ���ʱ�����еĶ�Ӧ������Ϊ��ɫ
				lOffsetJudge = i*wBytesPerLine + (j-1)*3;
				if (*(lpData + lOffsetJudge) ==0)
				{
					*(lpTemp + lOffset++) = 0;
					*(lpTemp + lOffset++) = 0;
					*(lpTemp + lOffset++) = 0;
					continue;
				}
				//�����ұߵĵ�
				lOffsetJudge =i*wBytesPerLine + (j+1)*3;
				if (*(lpData + lOffsetJudge) ==0)
				{
					*(lpTemp + lOffset++) = 0;
					*(lpTemp + lOffset++) = 0;
					*(lpTemp + lOffset++) = 0;
					continue;
				}

				//����������ߵĵ㶼�ǰ�ɫ�ѵ�����Ϊ��ɫ
				lOffset = i*wBytesPerLine + j*3;
				*(lpTemp + lOffset)   = 255;
				*(lpTemp + lOffset+1) = 255;
				*(lpTemp + lOffset+2) = 255;
				
			}
			//�����ǰ��Ϊ��ɫ�������ʱ�����ж�Ӧ������Ϊ��ɫ
			else
			{
				*(lpTemp + lOffset)   = 0;
				*(lpTemp + lOffset+1) = 0;
				*(lpTemp + lOffset+2) = 0;
			}
		}
		//��ͼ�����ܵĵ�����Ϊ��ɫ
		for(i=0; i<height; i++)
		{
			lOffset = i*wBytesPerLine;
				*(lpTemp + lOffset)   = 0;
				*(lpTemp + lOffset+1) = 0;
				*(lpTemp + lOffset+2) = 0;

		}
		
		for(i=0; i<height; i++)
		{
			lOffset = i*wBytesPerLine + (width-1)*3;
				*(lpTemp + lOffset)   = 0;
				*(lpTemp + lOffset+1) = 0;
				*(lpTemp + lOffset+2) = 0;

		}

	for (i=0; i<width; i++)
		{
			lOffset = i*3;
				*(lpTemp + lOffset)   = 0;
				*(lpTemp + lOffset+1) = 0;
				*(lpTemp + lOffset+2) = 0;

		}
	
	for (i=0; i<width; i++)
		{
			lOffset = (height-1)*wBytesPerLine + i*3;
				*(lpTemp + lOffset)   = 0;
				*(lpTemp + lOffset+1) = 0;
				*(lpTemp + lOffset+2) = 0;

		}
	//����ʱ����ĵ㿽����ԭ�����
	memcpy(lpData,lpTemp,wBytesPerLine*height);
	delete [] lpTemp;
	::GlobalUnlock((HGLOBAL) m_hDIB);

}

void CFaceDetectionDoc::DeleteFalseArea()
{
	int PixelNum[255];
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	int width,height;
	LPBYTE lpData;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	long lOffset;	
	//��ʼ�������ۼ�����
	for (int i=0; i<255; i++)
	{
		PixelNum[i] = 0;
	}	
	int calNum =1;
	for (i=0; i<height; i++)
		for (int j=0; j<width; j++)
		{
			lOffset = i*wBytesPerLine + j*3;
			//�������Ϊ��ɫ
			if (*(lpData + lOffset)==255)
			{	
				//�ݹ�ͳ�Ƹ����������İ�ɫ�����ص����
				RecursiveCal(lpData, i,j,wBytesPerLine, PixelNum[calNum],calNum);
				calNum++;
			}
		}
		
		for (i=0; i<calNum; i++)
		{	
			//������ص����С��һ����Ŀ��������־����Ϊ0
			if (PixelNum[i] < AREAPIXEL)
			{
				PixelNum[i] = 0;
			}
		}
		//�����ѭ�����ݱ�־�����������趨ͼ�����ɫ
		for(i=0; i<height; i++)
			for (int j=0; j<width; j++)
			{
				lOffset = i*wBytesPerLine + j*3;
				int num = *(lpData + lOffset);
				//�����ǰ�㲻�Ǻ�ɫ��
				if(num != 0)
				{	
					//�����־����Ϊ0��������Ϊ��ɫ
					if(PixelNum[num] == 0)
					{
						*(lpData+lOffset++) =0;
						*(lpData+lOffset++) =0;
						*(lpData+lOffset++) =0;
					}
					//��������Ϊ��ɫ
					else
					{
						*(lpData+lOffset++) =255;
						*(lpData+lOffset++) =255;
						*(lpData+lOffset++) =255;
					}
				}
			}
}
void CFaceDetectionDoc::RecursiveCal(LPBYTE lpData, int y, int x, WORD wBytesPerLine, int &pixelNum, int num)
{	
	long lOffset;
	
	lOffset = y*wBytesPerLine + x*3;
	//�����ǰ��Ϊ��ɫ��
	if(*(lpData+lOffset) == 255)
	{	
		//�ѵ�ǰ���С���ó�Ϊ���ֵ
		*(lpData+lOffset++) = num;
		*(lpData+lOffset++) = num;
		*(lpData+lOffset++) = num;
		//���ظ�����һ
		pixelNum++;
	
	int tempx;
	int tempy;
	
	//�ݹ鵱ǰ������ĵ�
	tempy = y-1;
	tempx = x;
	RecursiveCal(lpData,tempy,tempx,wBytesPerLine,pixelNum,num);
	
	//����ĵ�
	tempy = y+1;
	tempx = x;
	RecursiveCal(lpData,tempy,tempx,wBytesPerLine,pixelNum,num);
	
	//��ߵĵ�
	tempy = y;
	tempx = x-1;
	RecursiveCal(lpData,tempy,tempx,wBytesPerLine,pixelNum,num);
	//�ұߵĵ�
	tempy = y;
	tempx = x+1;
	RecursiveCal(lpData,tempy,tempx,wBytesPerLine,pixelNum,num);
	
	}

}

void  CFaceDetectionDoc::FaceLocate(CRect faceLocation[10], int &faceNum)
{	

	HANDLE  hDIBTemp;
	hDIBTemp = CopyHandle(m_hDIB);
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	//���浱ǰ����
	int width,height;
	LPBYTE lpData;
	long wBytesPerLine;
	wBytesPerLine = lLineBytesMulspec;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	long lOffset;
	//������Ŀ��ʼ��Ϊ0
	faceNum =0;
	for(int k=0; k<10; k++)
	{	
		//��ʼ������
		faceLocation[k].bottom = -1;
		faceLocation[k].top = height;
		faceLocation[k].right = -1;
		faceLocation[k].left = width;
	}

	for(int i=0; i<height; i++)
		for (int j=0; j<width; j++)
		{	
			//ƫ��
			lOffset = i*wBytesPerLine + j*3;
			int num;
			//��ǰ�����ֵ
			num = *(lpData + lOffset);
			if (num !=0)//���Ǻ�ɫ
			{	
				//�ݹ����
				RecursiveLocateRect(lpData, wBytesPerLine, i, j, num, faceLocation[faceNum]);
				faceNum++;
			}
		}
	::GlobalUnlock((HGLOBAL) m_hDIB);
	::GlobalFree((HGLOBAL) m_hDIB);
	//��ֵ��ԭ 
	m_hDIB = (HDIB)CopyHandle(hDIBTemp);
	LPSTR lpDIB2 = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);
	lpData = (unsigned char*)::FindDIBBits(lpDIB2);
	for (i=0; i<faceNum; i++)
		for (int j=faceLocation[i].top; j<faceLocation[i].bottom; j++)
		{	
			//�ѵõ���������������ɫ���α�ע��������ֱ��������
			lOffset = j*wBytesPerLine + faceLocation[i].left*3;
			*(lpData + lOffset++) = 0;
			*(lpData + lOffset++) = 255;
			*(lpData + lOffset++) = 0;
			lOffset = j*wBytesPerLine + faceLocation[i].right*3;
			*(lpData + lOffset++) = 0;
			*(lpData + lOffset++) = 255;
			*(lpData + lOffset++) = 0;

		}

		for (i=0; i<faceNum; i++)
		for (int j=faceLocation[i].left; j<faceLocation[i].right; j++)
		{	
			//����ˮƽ��������α�
			lOffset = faceLocation[i].top*wBytesPerLine + j*3;
			*(lpData + lOffset++) = 0;
			*(lpData + lOffset++) = 255;
			*(lpData + lOffset++) = 0;
			lOffset = faceLocation[i].bottom*wBytesPerLine + j*3;
			*(lpData + lOffset++) = 0;
			*(lpData + lOffset++) = 255;
			*(lpData + lOffset++) = 0;

		}
	
		::GlobalFree((HGLOBAL)hDIBTemp);	
		::GlobalUnlock((HGLOBAL) m_hDIB);
}

HANDLE CFaceDetectionDoc::CopyHandle( HANDLE  hSrc)
{	
	HANDLE  hDst;
	LPBITMAPINFOHEADER lpbi;
	int width,height;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock(hSrc);
	width = lpbi->biWidth;
	height = lpbi->biHeight;
	hDst = ::GlobalAlloc(GMEM_MOVEABLE,lpbi->biSize+lpbi->biSizeImage);
	if(!hDst)
		return NULL;
	LPBYTE lpDest;
	lpDest = (LPBYTE)::GlobalLock(hDst);
	memcpy(lpDest,(LPBYTE)lpbi,lpbi->biSize+lpbi->biSizeImage);
	::GlobalUnlock(hSrc);
	::GlobalUnlock(hDst);
	return hDst;
}
void CFaceDetectionDoc::RecursiveLocateRect(LPBYTE lpData,WORD wBytesPerLine, int y, int x, int num, CRect &faceRect)
{	
	long lOffset;
	//�õ�ƫ��
	lOffset = y*wBytesPerLine + x*3;
	//��ֵ�ж�
	if(*(lpData + lOffset) == num)
	{	
		//������ɫΪ��ɫ
		*(lpData + lOffset++) = 0;
		*(lpData + lOffset++) = 0;
		*(lpData + lOffset++) = 0;
		//�޸ľ��ε����������ĸ���λ��
		if(faceRect.bottom < y)
		{
			faceRect.bottom = y;
		}

		if(faceRect.top > y)
		{
			faceRect.top = y;
		}

		if(faceRect.right < x)
		{
			faceRect.right = x;
		}

		if(faceRect.left > x)
		{
			faceRect.left = x;
		}
		//�������ҵ��ñ��������������ж�
		RecursiveLocateRect(lpData, wBytesPerLine, y-1, x, num,faceRect);
		RecursiveLocateRect(lpData, wBytesPerLine, y+1, x, num, faceRect);
		RecursiveLocateRect(lpData, wBytesPerLine, y, x-1, num, faceRect);
		RecursiveLocateRect(lpData, wBytesPerLine, y, x+1, num, faceRect);
	}
	
}
void CFaceDetectionDoc::EyeMapR(LPBYTE lpRgb, const LPBYTE lpYcc,  WORD wBytesPerLine, CRect faceLocation)
{
	long lOffset;
	int cr;
	int cb;

	for(int i=faceLocation.top; i<=faceLocation.bottom; i++)
		for (int j=faceLocation.left; j<=faceLocation.right; j++)
		{
			lOffset =i*wBytesPerLine + j*3;
			cr = *(lpYcc + lOffset +1);
			cb = *(lpYcc + lOffset +2);
			*(lpRgb + lOffset++) = cr;
			*(lpRgb + lOffset++) = cr;
			*(lpRgb + lOffset++) = cr;
		}
	
}
void CFaceDetectionDoc::Crmap()
{
	LPBYTE lpRB;
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	int width,height;
	LPBYTE lpData;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	lpRB = new BYTE[wBytesPerLine * height];
	RgbtoYcb(m_hDIB,lpRB);
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	EyeMapR(lpData,lpRB,wBytesPerLine,CRect(0,0,width-1,height-1));
	::GlobalUnlock((HGLOBAL) m_hDIB);
}
void CFaceDetectionDoc::EyeMapC(LPBYTE lpRgb, const LPBYTE lpYcc,  WORD wBytesPerLine, CRect faceLocation)
{
	long lOffset;
	int cr;
	int cb;
	//���ݴ������ľ�����������۾���ɫ��ƥ��
	for(int i=faceLocation.top; i<=faceLocation.bottom; i++)
		for (int j=faceLocation.left; j<=faceLocation.right; j++)
		{	
			//�õ�Cr��Cb��ֵ
			lOffset = i*wBytesPerLine + j*3;
			cr = *(lpYcc + lOffset +1);
			cb = *(lpYcc + lOffset +2);
			//��־
			bool lab;
			//�ж�Cb��������ֵ�����޸ı�־
			int cmap = cb -120;//116 ;
			if(cmap >-1 && cmap <4)
				lab = true;
			else
				lab = false;
			//�ж�Cr��������ֵ�����޸ı�־
			 cmap =  cr- 143;//144  ;
			if(cmap <=-2 || cmap>= 2)
			{
				lab = false;
				
			}
			//���ݱ�־�趨ͼ����ɫ
			if(lab)
				cmap = 255;
			else
				cmap = 0;
			//����ͼ����ɫ
			*(lpRgb + lOffset++) = cmap;
			*(lpRgb + lOffset++) = cmap;
			*(lpRgb + lOffset++) = cmap;
		}	
}
void CFaceDetectionDoc::EyeMappingC()//����EyeMapC���������۾���ɫ��ƥ��
{
	LPBYTE lpYcc2;
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	int width,height;
	LPBYTE lpData;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	lpYcc2 = new BYTE[wBytesPerLine * height];
	RgbtoYcb( m_hDIBtemp,lpYcc2);//ת��ΪYCC�ռ�
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	//memcpy(lpData,gDib.YcctoRgb(lpYcc,gwBytesPerLine,gheight,gwidth),gwBytesPerLine*gheight);
	EyeMapC(lpData,lpYcc2,wBytesPerLine,CRect(0,0,width-1,height-1));
	for(int i=0; i<height; i++)
		for (int j=0; j<width; j++)
		{
			long lOffset;
			lOffset = i*wBytesPerLine + j*3;
			if(*(lpData + lOffset) == 255)
			{
				emymapc[i][j] = true;
			}
		}
	::GlobalUnlock((HGLOBAL) m_hDIB);
}

void CFaceDetectionDoc::EyeMapL(LPBYTE lpRgb, WORD wBytesPerLine, CRect faceLocation)
{
	int r;
	int g;
	int b;
	int gray ;
	long lOffset;
	//�����ѭ��ʵ���۾�������ƥ��
	for (int i=faceLocation.top; i<=faceLocation.bottom; i++)
		for (int j=faceLocation.left; j<=faceLocation.right; j++)
		{
			lOffset = i*wBytesPerLine + j*3;
			//�õ�rgbֵ
			b = *(lpRgb + lOffset);
			g = *(lpRgb + lOffset+1);
			r = *(lpRgb + lOffset+2);
			//����õ��Ҷ�
			gray = (r*10+g*30+b*60)/100;
			//�����۾��������������趨ͼ�����ֵ
			if(/*100*/120<gray && /*125*/160>gray)
				gray =255;
			else
				gray = 0;
			*(lpRgb + lOffset++) = gray;
			*(lpRgb + lOffset++) = gray;
			*(lpRgb + lOffset++) = gray;
		}
}
void CFaceDetectionDoc::EyeMappingL()//����EyeMapC���������۾���ɫ��ƥ��
{
	LPBYTE lpYcc2;
	LPBYTE lpDataR,lpDataD;
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	int width,height;
	LPBYTE lpData;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);
	LPSTR lpDIBtemp = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIBtemp);
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	lpYcc2 = new BYTE[wBytesPerLine * height];
	lpDataR = (unsigned char*)::FindDIBBits(lpDIBtemp);
	RgbtoYcb(m_hDIBtemp,lpYcc2);
	lpDataD = (unsigned char*)::FindDIBBits(lpDIB);
	memcpy(lpDataD,lpDataR,wBytesPerLine*height);
	EyeMapL(lpDataD,wBytesPerLine, CRect(0,0,width-1,height-1));
	for (int i=0; i<height; i++)
		for (int j=0; j<width; j++)
		{
			long lOffset;
			lOffset =  i*wBytesPerLine + j*3;
			if(*(lpDataD + lOffset) == 255)
			{
				emymapl[i][j] = true;
			}
		}
	::GlobalUnlock((HGLOBAL) m_hDIB);
	::GlobalUnlock((HGLOBAL) m_hDIBtemp);
}

void CFaceDetectionDoc::Eyemap() 
{
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	int width,height;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	LPBYTE lpData;
	long lOffset;
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	for (int i=0; i<height; i++)
		for (int j=0; j<width; j++)
		{
			lOffset =  i*wBytesPerLine + j*3;
			if( emymapc[i][j] && emymapl[i][j])
			{
				*(lpData + lOffset++) = 255;
				*(lpData + lOffset++) = 255;
				*(lpData + lOffset++) = 255;
			}

			else
			{
				*(lpData + lOffset++) = 0;
				*(lpData + lOffset++) = 0;
				*(lpData + lOffset++) = 0;
			}
		}
}
void CFaceDetectionDoc::DeleteFasleEye(CRect facelocation)
{
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	int width,height;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	LPBYTE lpData;
	long lOffset;
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	for (int i=0; i<height; i++)
		for (int j=0; j<width; j++)
		{
			lOffset =  i*wBytesPerLine + j*3;
			if(*(lpData + lOffset) == 255)
			{
				if(i<(facelocation.bottom+facelocation.top)/2)
				{
					*(lpData + lOffset++) = 0;
					*(lpData + lOffset++) = 0;
					*(lpData + lOffset++) = 0;
				}
			}
		}
	::GlobalUnlock((HGLOBAL) m_hDIB);
}

void CFaceDetectionDoc::EyeCenter(CPoint &eye1, CPoint &eye2)
{
	
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	int width,height;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	LPBYTE lpData;
	long lOffset;
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	
	int pixelnum =0;
	int num =0;
	CRect faceLocation(0,0,width-1,height-1);
	//������������
	for(int i=faceLocation.top; i<faceLocation.bottom; i++)
		for (int j=faceLocation.left; j<faceLocation.right; j++)
		{
			lOffset =  i*wBytesPerLine + j*3;
			//��ɫ��
			if(*(lpData + lOffset) == 255)
				//�ݹ�ͳ�����ز��޸�����ֵ
				RecursiveCal(lpData,i,j,wBytesPerLine,pixelnum,++num);
		}
		//��ʼ���۾�������
		eye1.x =0;
		eye1.y =0;
		eye2.x =0;
		eye2.y =0;
		//��ʼ�����ص����
		int eye1count=0;
		int eye2count =0;
		for (i=faceLocation.top; i<faceLocation.bottom; i++)
			for (int j=faceLocation.left; j<faceLocation.right; j++)
			{
				lOffset =  i*wBytesPerLine + j*3;
				//������ص����ֵΪ1
				if(*(lpData + lOffset) == 1)
				{	
					//�۾�1�ĺ��������������ϵ�ǰ�������ֵ
					eye1.x +=j;
					eye1.y +=i;
					eye1count++;
					//�ѵ�ǰ��ĳɰ�ɫ
					*(lpData + lOffset++) = 255;
					*(lpData + lOffset++) = 255;
					*(lpData + lOffset++) = 255;

				}
				//�����ǰ���ص���ֵΪ2
				else if(*(lpData + lOffset) == 2)
				{	
					//�۾�2�ĺ��������������ϵ�ǰ�������ֵ
					eye2.x +=j;
					eye2.y +=i;
					//���ص������һ
					eye2count++;
					//�ѵ�ǰ������Ϊ��ɫ
					*(lpData + lOffset++) = 255;
					*(lpData + lOffset++) = 255;
					*(lpData + lOffset++) = 255;
				}
			}
			//�����۾������ĵ�����
			eye1.x /=eye1count;
			eye1.y /=eye1count;
			eye2.x /=eye2count;
			eye2.y /=eye2count;
			//�����ĵ�����Ϊ��ɫ
			lOffset = eye1.y*wBytesPerLine + eye1.x*3; 
			*(lpData + lOffset++) = 0;
			*(lpData + lOffset++) = 255;
			*(lpData + lOffset++) = 0;

			lOffset = eye2.y*wBytesPerLine + eye2.x*3; 
			*(lpData + lOffset++) = 0;
			*(lpData + lOffset++) = 255;
			*(lpData + lOffset++) = 0;
	::GlobalUnlock((HGLOBAL) m_hDIB);
}

void  CFaceDetectionDoc::FunctionMouseMap(LPBYTE lpRgb, const LPBYTE lpYcc,  WORD wBytesPerLine, CRect faceLocation)
{	
	//�����ѭ����������������ʵ����͵�ƥ��
	for (int i=faceLocation.top; i<faceLocation.bottom; i++)
		for (int j=faceLocation.left; j<faceLocation.right; j++)
		{	
			//�õ�ƫ��
			long lOffset = i*wBytesPerLine + j*3;
			//�õ�cr��cb����ֵ
			int cr = *(lpYcc+lOffset+1);
			int cb = *(lpYcc+lOffset+2);
			//��־
			bool lab;
			int mapm;
			//����cr����ֵ�趨��־
			cr = cr-157;//143;
			if(cr <-6 || cr>6)
			{
				cr = 0;
				
			}
			cr *=cr;
			
			if(cr>16)
				 lab = true;
			else
				lab = false;
			//����cb��ʱֵ�趨��־
			cb= cb-118;//120;
			if(cb<-5 || cb >5)
				
			{
				cb = 0;
				if(lab = true)
					lab = false;
			}
			//���cr��cb������ֵ�����趨�ķ�Χ֮�ڣ����趨��ɫλ��ɫ�������ɫ
			if(lab)
				mapm = 255;
			else
				mapm = 0;
			 
			*(lpRgb + lOffset++) = mapm;
			*(lpRgb + lOffset++) = mapm;
			*(lpRgb + lOffset++) = mapm;

			
		}
}
void  CFaceDetectionDoc::MouseMap()
{
	LPBYTE lpYcc2;
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	int width,height;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	LPBYTE lpData;
	long lOffset;
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	lpYcc2 = new BYTE[wBytesPerLine * height];
	RgbtoYcb(m_hDIBtemp,lpYcc2);
	FunctionMouseMap(lpData,lpYcc2,wBytesPerLine,CRect(0,0,width-1,height-1));
	::GlobalUnlock((HGLOBAL) m_hDIB);

}
void CFaceDetectionDoc::DeleteScatePoint()
{	
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	int width,height;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	LPBYTE lpData;
	long lOffset;
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	for (int i=0; i<height; i++)
		for(int j=0; j<width; j++)
		{	
			//�õ�ƫ��
			lOffset =i*wBytesPerLine + j*3;
			//�����ǰ��Ϊ��ɫ��
			if(*(lpData + lOffset) == 255)
			{	
				//�趨�ж�����
				for(int ii = 0;ii<ImgRange;ii++)
					for (int jj=0; jj<ImgRange; jj++)
						this->lab[ii][jj] = false;
					//�趨�жϳ���
					lenth=0;
					//�ж��Ƿ�Ϊ��ɢ��
				bool judge = IsScaterPoint(j, i, width,height,lpData,wBytesPerLine,3,lab);
				if(!judge)
				{	
					//����ɢ����Ѹõ�����Ϊ��ɫ
					*(lpData + lOffset++) = 0;
					*(lpData + lOffset++) = 0;
					*(lpData + lOffset++) = 0;
				}
			}
		}
	::GlobalUnlock((HGLOBAL) m_hDIB);
}
BOOL CFaceDetectionDoc::IsScaterPoint(int x, int y, int width, int height, LPBYTE lpData,WORD wBytesPerLine,  int threshold,bool lab[m_HEIGHT][m_WIDTH])
{
	long lOffset;
	//�õ����ݵ�ƫ��
	lOffset = y*wBytesPerLine + x*3;
	//�жϸõ��Ƿ�Ϊ��ɫ�Լ��Ƿ�������
	if(*(lpData+lOffset) == 255 && lab[y][x] == false)
	{	
		//�����ȼ�һ
		lenth++;
		//���ı�־λ
		lab[y][x] = true;
		//��������ȴﵽ�ٽ�ֵ�򷵻���
	if(lenth >= threshold)
		return true;
	//���ұߵ�ı߽��ж��Լ���־λ�ж�
	if(x+1<width && lab[y][x+1] == false)
	{	
		//�ݹ���ñ����������ұߵĵ�����ж�
		IsScaterPoint(x+1,y,width,height,lpData,wBytesPerLine,threshold,lab);
		if(lenth>=threshold)
			return true;
		
	}
	//������ߵĵ�
	if(x-1>=0 && lab[y][x-1] == false)
	{
		(IsScaterPoint(x-1,y,width,height,lpData,wBytesPerLine,threshold,lab));
		if(lenth>=threshold)
			return true;
		
	}
	//��������ĵ�
	if(y-1>=0 && lab[y-1][x]==false)
	{
		(IsScaterPoint(x,y-1,width,height,lpData,wBytesPerLine,threshold,lab));
		if(lenth>=threshold)
			return true;
		
	}
	//��������ĵ�
	if(y+1<height && lab[y+1][x]==false)
	{	(IsScaterPoint(x,y+1,width,height,lpData,wBytesPerLine,threshold,lab));
			if(lenth>=threshold)
			return true;
			
	}
	//�������µĵ�
	if(y+1<height  && x+1 <width && lab[y+1][x+1]==false)
	{	(IsScaterPoint(x+1,y+1,width,height,lpData,wBytesPerLine,threshold,lab));
			if(lenth>=threshold)
			return true;
			
	}
	//�������µĵ�
	if(y+1<height && x-1 >=0 && lab[y+1][x-1]==false)
	{	(IsScaterPoint(x-1,y+1,width,height,lpData,wBytesPerLine,threshold,lab));
			if(lenth>=threshold)
			return true;
			
	}
	//�������ϵĵ�
	if(y-1>=0 && x-1 >=0 &&lab[y-1][x-1]==false)
	{	(IsScaterPoint(x-1,y-1,width,height,lpData,wBytesPerLine,threshold,lab));
			if(lenth>=threshold)
			return true;
			
	}
	//�������ϵĵ�
	if(y-1<height && x+1<width && lab[y+1][x]==false)
	{	(IsScaterPoint(x+1,y-1,width,height,lpData,wBytesPerLine,threshold,lab));
			if(lenth>=threshold)
			return true;
			
	}
	}	
		//����ݹ���������ȴﲻ���ٽ�ֵ�����ؼ�
		return false;
}

void CFaceDetectionDoc::MouthCenter(CPoint &mouthLocation)
{
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIB);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIB);
	int width,height;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	LPBYTE lpData;
	long lOffset;
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	
	//������������������ۼ������������ص�x��y�����ص���
	int xnum = 0 ;
	int ynum = 0 ;
	int count = 0;
	CRect faceLocation(0,0,width-1,height-1);
	for (int i=faceLocation.top; i<faceLocation.bottom; i++)
		for (int j=faceLocation.left; j<faceLocation.right; j++)
		{
			lOffset = i*wBytesPerLine + j*3;
			//��ɫ��
			if(*(lpData + lOffset) == 255)
			{	
				//xֵ��
				xnum +=j;
				//yֵ��
				ynum +=i;
				//������
				count++;
			}
		}
		//�õ����ĵ�λ��
	mouthLocation.x = xnum/count;
	mouthLocation.y = ynum/count;
	
	//�����ĵ�����λ��ɫ
	lOffset = mouthLocation.y*wBytesPerLine + mouthLocation.x*3;
	*(lpData + lOffset++) =0;
	*(lpData + lOffset++) =255;
	*(lpData + lOffset++) =0;
	::GlobalUnlock((HGLOBAL) m_hDIB);
}
void CFaceDetectionDoc::EllipseFace(CPoint mouth, CPoint eye1, CPoint eye2,CRect faceLocation)
{
	LPBITMAPINFOHEADER lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock((HGLOBAL) m_hDIBtemp);//��ȡͷ�ļ�
	lLineBytesMulspec = WIDTHBYTES((lpbi->biWidth)*24);
	::GlobalUnlock((HGLOBAL) m_hDIBtemp);
	int width,height;
	long wBytesPerLine;
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIBtemp);	
	// ��ȡDIB���
	width= (int) ::DIBWidth(lpDIB);		
	// ��ȡDIB�߶�
	height= (int) ::DIBHeight(lpDIB);
	//�õ�ͼƬÿ�е�������ռ�ֽڸ���
	wBytesPerLine = lLineBytesMulspec;
	LPBYTE lpData;
	long lOffset;
	lpData = (unsigned char*)::FindDIBBits(lpDIB);
	
	//��dda�㷨��������
	DdaLine(mouth,eye1,lpData,wBytesPerLine);
	DdaLine(mouth,eye2,lpData,wBytesPerLine);
	DdaLine(eye1,eye2,lpData,wBytesPerLine);
	/*
	
	//��Բ�����ĵ��������������
	int ellipsecenter_x;
	int ellipsecenter_y;
	int ellipseFocusTop_x;
	int ellipseFocusTop_y;
	int ellipseFocusBottom_x;
	int ellipseFocusBottom_y;
	
	//�����۾�����͵����������Բ�����ĵ�����
	ellipsecenter_x = (eye1.x + eye2.x + mouth.x )/3;
	ellipsecenter_y = (eye1.y + eye2.y)/2 -abs(eye2.x - eye1.x)/2;

	//����Ľ���
	ellipseFocusTop_x = ellipsecenter_x;
	ellipseFocusBottom_x = ellipsecenter_x;

	//����Ľ���
	ellipseFocusTop_y =  ellipsecenter_y + (eye1.y +eye2.y)/2 -mouth.y;
	ellipseFocusBottom_y = ellipsecenter_y - ((eye1.y +eye2.y)/2 -mouth.y)+2;

	//����
	int a = (eye1.x-eye2.x)*2-2;
	
	for (int i=0; i<height; i++)
		for (int j=0; j<width; j++)
		{	
			//�õ�һ���㵽��������ľ����
			int lenth = sqrt(pow(j-ellipseFocusTop_x,2)+pow(i-ellipseFocusTop_y,2))
				+sqrt(pow(j-ellipseFocusBottom_x,2)+ pow(i-ellipseFocusBottom_y,2));
			//�жϾ�����볤��Ĺ�ϵ
			if(lenth<2*a+2 && lenth >2*a-2)
			{	
				//�ѵ�����Ϊ��ɫ
				lOffset =i*wBytesPerLine + j*3;
				*(lpData + lOffset++) = 0;
				*(lpData + lOffset++) = 255;
				*(lpData + lOffset++) = 0;
			}
		}*/
		for (int j=faceLocation[0].top; j<faceLocation[0].bottom; j++)
		{	
			//�ѵõ���������������ɫ���α�ע��������ֱ��������
			lOffset = j*wBytesPerLine + faceLocation[0].left*3;
			*(lpData + lOffset++) = 0;
			*(lpData + lOffset++) = 255;
			*(lpData + lOffset++) = 0;
			lOffset = j*wBytesPerLine + faceLocation[0].right*3;
			*(lpData + lOffset++) = 0;
			*(lpData + lOffset++) = 255;
			*(lpData + lOffset++) = 0;

		}

		for (j=faceLocation[0].left; j<faceLocation[0].right; j++)
		{	
			//����ˮƽ��������α�
			lOffset = faceLocation[0].top*wBytesPerLine + j*3;
			*(lpData + lOffset++) = 0;
			*(lpData + lOffset++) = 255;
			*(lpData + lOffset++) = 0;
			lOffset = faceLocation[0].bottom*wBytesPerLine + j*3;
			*(lpData + lOffset++) = 0;
			*(lpData + lOffset++) = 255;
			*(lpData + lOffset++) = 0;

		}
	

	::GlobalUnlock((HGLOBAL) m_hDIBtemp);
	m_hDIB = (HDIB)CopyHandle(m_hDIBtemp);

}


void CFaceDetectionDoc::DdaLine(CPoint from, CPoint end, LPBYTE lpData, WORD wBytesPerLine)
{	
	//x��y������
	float delta_x;
	float delta_y;
	//x��y������
	float x;
	float y;
	//x��y�ϵĲ�ֵ
	int dx;
	int dy;
	//�ܵĲ���
	int steps;
	int k;
	//�õ�x��y�Ĳ�ֵ
	dx = end.x - from.x;
	dy = end.y - from.y;
	//�ж�x��y�ϵĲ�ֵ��С��ȷ������
	if(abs(dx) > abs(dy))
	{
		steps = abs(dx);
	}
	else
	{
		steps = abs(dy);
	}

	//�õ�ÿ�������Ĵ�С
	delta_x = (float)dx / (float)steps;
	delta_y = (float)dy / (float)steps;
	//�趨x��y�����
	x = (float)from.x;
	y = (float)from.y;

	//�趨��ʼ�����ɫΪ��ɫ
	long lOffset = y*wBytesPerLine + x*3;
	*(lpData + lOffset++) = 0;
	*(lpData + lOffset++) = 255;
	*(lpData + lOffset++) = 0;

	//���ݼ���õ��Ĳ�������ֱ���ϵĵ�������ɫ
	for (k=1;k<=steps; k++)
	{	
		//x��y�ֱ���ϸ��Ե�����
		x+=delta_x;
		y+=delta_y;
		//���õ����ɫ
		lOffset = y*wBytesPerLine + x*3;
		*(lpData + lOffset++) = 0;
		*(lpData + lOffset++) = 255;
		*(lpData + lOffset++) = 0;
	}
	
}

void CFaceDetectionDoc::OnFileSaveAs(LPCTSTR lpszPathName) 
{
	CFile file;
	if(!file.Open(lpszPathName, CFile::modeCreate |// ���ļ�
	  CFile::modeReadWrite | CFile::shareExclusive))
	{
		return;// ����FALSE
	}
	BOOL bSuccess = FALSE;	
	bSuccess = ::SaveDIB(m_hDIB, file);	// ����ͼ��	
	file.Close();// �ر��ļ�	
	SetModifiedFlag(FALSE);// �����ͱ��ΪFALSE
	if (!bSuccess)
	{			
		AfxMessageBox("����BMPͼ��ʱ����");// ��ʾ����
	}	
	
}
