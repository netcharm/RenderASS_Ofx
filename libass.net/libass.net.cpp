// 这是主 DLL 文件。

#include "stdafx.h"
#include <tchar.h>
#include <windows.h>

#include "libass.net.h"

#include <iconv.h>
#include <ass.h>
#include "libass_helper.h"

#ifndef OfxRGBAColourB
typedef struct OfxRGBAColourB {
	unsigned char b, g, r, a;
}OfxRGBAColourB;
#endif

const int bDepth = 4; /// red, green, blue
const int fileHeaderSize = 14;
const int infoHeaderSize = 108;

const char* buf;

#define BUFSIZE 1024

#define _R(c)  ((c)>>24)
#define _G(c)  (((c)>>16)&0xFF)
#define _B(c)  (((c)>>8)&0xFF)
#define _A(c)  ((c)&0xFF)

void changePixelColorOrder(const unsigned char *image, int width, int height);
inline array<unsigned char>^ createBitmapFileHeaderClr(int width, int height);
inline array<unsigned char>^  createBitmapInfoHeaderClr(int width, int height);
Image^ generateBitmapImageClr(unsigned char *image, int width, int height);

void changePixelColorOrder(const unsigned char *image, int width, int height) {
	int dstRowBytes = width*bDepth;

	OfxRGBAColourB *dst = (OfxRGBAColourB *)image;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			try
			{
				unsigned int a = dst->a;
				unsigned int r = dst->r;
				unsigned int g = dst->g;
				unsigned int b = dst->b;

				dst->a = a; ///alpha
				dst->r = b; ///red
				dst->g = g; ///green
				dst->b = r; ///blue				
			}
			catch (const std::exception&)
			{

			}
			dst++;
		}
	}
}

inline array<unsigned char>^ createBitmapFileHeaderClr(int width, int height) {
	int fileSize = fileHeaderSize + infoHeaderSize + height * width * bDepth;

	array<unsigned char>^ fileHeader = gcnew array<unsigned char>(14) {
		0, 0, /// signature
			0, 0, 0, 0, /// image file size in bytes
			0, 0, 0, 0, /// reserved
			0, 0, 0, 0, /// start of pixel array
	};

	fileHeader[0] = (unsigned char)('B');
	fileHeader[1] = (unsigned char)('M');
	fileHeader[2] = (unsigned char)(fileSize);
	fileHeader[3] = (unsigned char)(fileSize >> 8);
	fileHeader[4] = (unsigned char)(fileSize >> 16);
	fileHeader[5] = (unsigned char)(fileSize >> 24);
	fileHeader[10] = (unsigned char)(fileHeaderSize + infoHeaderSize);

	return fileHeader;
}

inline array<unsigned char>^  createBitmapInfoHeaderClr(int width, int height) {
	array<unsigned char>^  infoHeader = gcnew array<unsigned char>(infoHeaderSize) {
		0, 0, 0, 0, /// header size
			0, 0, 0, 0, /// image width
			0, 0, 0, 0, /// image height
			0, 0, /// number of color planes
			0, 0, /// bits per pixel
			0, 0, 0, 0, /// compression
			0, 0, 0, 0, /// image size
			0, 0, 0, 0, /// horizontal resolution
			0, 0, 0, 0, /// vertical resolution
			0, 0, 0, 0, /// colors in color table
			0, 0, 0, 0, /// important color count

			0, 0, 0xFF, 0, /// Red channel bit mask (valid because BI_BITFIELDS is specified)
			0, 0xFF, 0, 0, /// Green channel bit mask (valid because BI_BITFIELDS is specified) 
			0xFF, 0, 0, 0, /// Blue channel bit mask (valid because BI_BITFIELDS is specified)
			0, 0, 0, 0xFF, /// Alpha channel bit mask 	
			0x20, 0x6E, 0x69, 0x57, /// LCS_WINDOWS_COLOR_SPACE 
			0, 0, 0, 0, 0, 0, 0, 0, 0, /// CIEXYZTRIPLE Color Space endpoints Unused for LCS "Win " or "sRGB" 
			0, 0, 0, 0, /// Red Gamma Unused for LCS "Win " or "sRGB"
			0, 0, 0, 0, /// Green Gamma Unused for LCS "Win " or "sRGB" 
			0, 0, 0, 0, /// Blue Gamma Unused for LCS "Win " or "sRGB"
	};

	int lineSize = width*bDepth;
	int paddingSize = (4 - (lineSize) % 4) % 4;
	int imageSize = (lineSize + paddingSize)*height;

	infoHeader[0] = (unsigned char)(infoHeaderSize);

	infoHeader[4] = (unsigned char)(width);
	infoHeader[5] = (unsigned char)(width >> 8);
	infoHeader[6] = (unsigned char)(width >> 16);
	infoHeader[7] = (unsigned char)(width >> 24);

	infoHeader[8] = (unsigned char)(height);
	infoHeader[9] = (unsigned char)(height >> 8);
	infoHeader[10] = (unsigned char)(height >> 16);
	infoHeader[11] = (unsigned char)(height >> 24);

	infoHeader[12] = (unsigned char)(1);

	infoHeader[14] = (unsigned char)(bDepth * 8);

	infoHeader[16] = (unsigned char)(3);

	infoHeader[20] = (unsigned char)(imageSize);
	infoHeader[21] = (unsigned char)(imageSize >> 8);
	infoHeader[22] = (unsigned char)(imageSize >> 16);
	infoHeader[23] = (unsigned char)(imageSize >> 24);

	infoHeader[24] = (unsigned char)(0x13);
	infoHeader[25] = (unsigned char)(0x0B);

	infoHeader[28] = (unsigned char)(0x13);
	infoHeader[29] = (unsigned char)(0x0B);

	return infoHeader;
}

Image^ generateBitmapImageClr(unsigned char *image, int width, int height) {

	int lineSize = width*bDepth;
	int imageSize = lineSize*height;
	int paddingSize = (4 - (lineSize) % 4) % 4;

	array<unsigned char>^ fileHeader = createBitmapFileHeaderClr(width, height);
	array<unsigned char>^ infoHeader = createBitmapInfoHeaderClr(width, height);
	array<unsigned char>^ padding = gcnew array<unsigned char>(3) { 0, 0, 0 };

	array<unsigned char> ^lines = gcnew array<unsigned char>(imageSize);
	Marshal::Copy((IntPtr)image, lines, 0, imageSize);

	MemoryStream ^ms = gcnew MemoryStream();

	ms->Write(fileHeader, 0, fileHeader->Length);
	ms->Write(infoHeader, 0, infoHeader->Length);

	for (int i = 0; i<height; i++) {
		try {
			ms->Write(lines, i*lineSize, lineSize);
			ms->Write(padding, 1, paddingSize);
		}
		finally
		{
			// Free the unmanaged memory.
		}
	}
	ms->Seek(0, SeekOrigin::Begin);
	return gcnew Bitmap(ms);;
}

AssRender* ass;

libass::Render::Render(void)
{
	ass = new AssRender((ASS_Hinting)ASS_HINTING_NONE, 1.0, "UTF-8");
}

libass::Render::Render(ASSHinting hints, double scale, String^ encoding)
{
	ass = new AssRender((ASS_Hinting)hints, scale, "UTF-8");
}

libass::Render::~Render()
{
	if(ass != NULL) ass->~AssRender();
}

bool libass::Render::SetDefaultFont(String^ fontname, int fontsize)
{
	char* font = (char*)(void*)Marshal::StringToHGlobalAnsi(fontname);
	bool result = ass->SetDefaultFont(font, fontsize);
	Marshal::FreeHGlobal(IntPtr(font));

	return result;
}

bool libass::Render::SetDefaultFontName(String^ fontname)
{
	char* font = (char*)(void*)Marshal::StringToHGlobalAnsi(fontname);
	bool result = ass->SetDefaultFontName(font);
	Marshal::FreeHGlobal(IntPtr(font));

	return result;
}

bool libass::Render::SetDefaultFontSize(int fontsize)
{
	return ass->SetDefaultFontSize(fontsize);
}

bool libass::Render::SetDefaultFontColor(Color^ color)
{
	RGBA rgba;
	rgba.a = color->A;
	rgba.r = color->R;
	rgba.g = color->G;
	rgba.b = color->B;
	return ass->SetDefaultFontColor(rgba);
}

bool libass::Render::SetDefaultFontColorAlt(Color^ color)
{
	RGBA rgba;
	rgba.a = color->A;
	rgba.r = color->R;
	rgba.g = color->G;
	rgba.b = color->B;
	return ass->SetDefaultFontColorAlt(rgba);
}

bool libass::Render::SetDefaultOutlineColor(Color^ color)
{
	RGBA rgba;
	rgba.a = color->A;
	rgba.r = color->R;
	rgba.g = color->G;
	rgba.b = color->B;
	return ass->SetDefaultOutlineColor(rgba);
}

bool libass::Render::SetDefaultBackColor(Color^ color)
{
	RGBA rgba;
	rgba.a = color->A;
	rgba.r = color->R;
	rgba.g = color->G;
	rgba.b = color->B;
	return ass->SetDefaultBackColor(rgba);
}

bool libass::Render::SetDefaultBold(int bold)
{
	return ass->SetDefaultBold(bold);
}

bool libass::Render::SetDefaultItalic(int italic)
{
	return ass->SetDefaultItalic(italic);
}

bool libass::Render::SetDefaultUnderline(int underline)
{
	return ass->SetDefaultUnderline(underline);
}

bool libass::Render::SetDefaultStrikeOut(int strikeout)
{
	return ass->SetDefaultStrikeOut(strikeout);
}

bool libass::Render::SetDefaultBorderStyle(int borderstyle)
{
	return ass->SetDefaultBorderStyle(borderstyle);
}

bool libass::Render::SetDefaultOutline(int outline)
{
	return ass->SetDefaultOutline(outline);
}

bool libass::Render::SetDefaultShadow(int shadow)
{
	return ass->SetDefaultShadow(shadow);
}

bool libass::Render::SetDefaultStyle(void)
{
	return ass->SetDefaultStyle();
}

bool libass::Render::SetDefaultStyle(String^ fontname, int fontsize, Color^ color, Color^ coloralt, Color^ outlinecolor, Color^ backcolor, int bold, int italic, int underline, int strikeout, int borderstyle, int outline, int shadow)
{
	RGBAColourD rgba;
	rgba.a = color->A;
	rgba.r = color->R;
	rgba.g = color->G;
	rgba.b = color->B;

	RGBAColourD rgba_alt;
	rgba_alt.a = coloralt->A;
	rgba_alt.r = coloralt->R;
	rgba_alt.g = coloralt->G;
	rgba_alt.b = coloralt->B;

	RGBAColourD rgba_outline;
	rgba_outline.a = outlinecolor->A;
	rgba_outline.r = outlinecolor->R;
	rgba_outline.g = outlinecolor->G;
	rgba_outline.b = outlinecolor->B;

	RGBAColourD rgba_back;
	rgba_back.a = outlinecolor->A;
	rgba_back.r = outlinecolor->R;
	rgba_back.g = outlinecolor->G;
	rgba_back.b = outlinecolor->B;

	char* font = (char*)(void*)Marshal::StringToHGlobalAnsi(fontname);
	bool result = ass->SetDefaultStyle(font, fontsize, 
		rgba, rgba_alt, rgba_outline, rgba_back,
		bold, italic, underline, strikeout, borderstyle, outline, shadow);
	Marshal::FreeHGlobal(IntPtr(font));

	return result;
}

bool libass::Render::SetUseDefaultStyle(int used)
{
	return ass->SetUseDefaultStyle(used);
}

bool libass::Render::SetSpace(int pixels)
{
	return ass->SetSpace(pixels);
}

bool libass::Render::SetSpace(double percentage)
{
	return ass->SetSpace(percentage);
}

bool libass::Render::SetPosition(int pixels)
{
	return ass->SetPosition(pixels);
}

bool libass::Render::SetPosition(double percentage)
{
	return ass->SetPosition(percentage);
}

bool libass::Render::SetMargin(int used)
{
	return ass->SetMargin(used);
}

bool libass::Render::SetMargin(int t, int b, int l, int r)
{
	return ass->SetMargin(t, b, l, r);
}

bool libass::Render::SetMargin(double t, double b, double l, double r)
{
	return ass->SetMargin(t, b, l, r);
}

bool libass::Render::SetMargin(int used, int t, const int b, int l, int r)
{
	return ass->SetMargin(used, t, b, l, r);
}

bool libass::Render::SetMargin(int used, double t, double b, double l, double r)
{
	return ass->SetMargin(used, t, b, l, r);
}

bool libass::Render::SetFontScale(double scale)
{
	return ass->SetFontScale(scale);
}

bool libass::Render::SetHints(ASS_Hinting hints)
{
	return ass->SetFontScale(hints);
}

bool libass::Render::Resize(int width, int height)
{
	renderWidth = width;
	renderHeight = height;
	return ass->Resize(width, height);
}

bool libass::Render::ReScale(double scale)
{
	return ass->ReScale(scale);
}

bool libass::Render::SetFPS(double fps)
{
	renderFPS = fps;
	return ass->SetFPS(fps);
}

double libass::Render::GetDuration(void)
{
	return ass->GetDuration();
}

double libass::Render::GetDuration(double start)
{
	return ass->GetDuration(start);
}

double libass::Render::GetFrames(void)
{
	return ass->GetFrames();
}

double libass::Render::GetFrames(double times)
{
	return ass->GetFrames(times);
}

double libass::Render::GetFrames(int framestart)
{
	return ass->GetFrames(framestart);
}

bool libass::Render::SetOffset(double offset)
{
	return ass->SetOffset(offset);
}

bool libass::Render::LoadAss(String ^ filename, String^ encoding)
{
	if (String::IsNullOrEmpty(encoding))
		encoding = "UTF-8";
	char* encodingname = (char*)(void*)Marshal::StringToHGlobalAnsi(encoding);
	char* assfilename = (char*)(void*)Marshal::StringToHGlobalAnsi(filename);
	bool result = ass->LoadAss(assfilename, encodingname);
	Marshal::FreeHGlobal(IntPtr(assfilename));
	Marshal::FreeHGlobal(IntPtr(encodingname));
	return result;
}

bool libass::Render::LoadAss(String ^ contents)
{
	System::Text::Encoding^ utf8 = System::Text::Encoding::UTF8;

	array<unsigned char>^ bytes = utf8->GetBytes(contents);
	int count = bytes->Length;
	char* lines = (char*)calloc(count, sizeof(const char));
	Marshal::Copy(bytes, 0, (IntPtr)lines, count);
	bool result = ass->LoadAss(lines, count, "UTF-8");
	free((void*)buf);
	return result;
}

bool libass::Render::LoadAss(Stream ^ stream)
{
	array<unsigned char>^ buf = gcnew array<unsigned char>(stream->Length);
	stream->Read(buf, 0, stream->Length);
	pin_ptr<unsigned char> pBuf = &buf[0];
	bool result = ass->LoadAss((char*)pBuf, stream->Length, "UTF-8");
	return result;
}

Image ^ libass::Render::GetAss(double frame)
{
	// TODO: 在此处插入 return 语句
	return GetAss(frame, renderWidth, renderHeight);
}

///
/// timestamp is timestamp value
///
Image ^ libass::Render::GetAss(TimeSpan^ time)
{
	// TODO: 在此处插入 return 语句
	double frame = time->TotalSeconds*renderFPS;
	return GetAss(frame, renderWidth, renderHeight);
}

Image ^ libass::Render::GetAss(double frame, int width, int height)
{
	Image^ img;

	renderWidth = width;
	renderHeight = height;
	
	ass->Resize(renderWidth, renderHeight);

	unsigned int count = renderWidth*renderHeight*renderDepth;
	buf = (const char*)calloc(count, sizeof(const char));

	OfxRGBAColourB *image = (OfxRGBAColourB *)buf;

	for (int h = 0; h<renderHeight; h++) {
		for (int w = 0; w<renderWidth; w++) {
			image->a = 0x00; ///alpha
			image->r = 0x00; ///red
			image->g = 0x00; ///green
			image->b = 0x00; ///blue
			image++;
		}
	}

	int c = ass->GetAss(frame, renderWidth, renderHeight, renderDepth, buf, true);
	if (c > 0) {
		changePixelColorOrder((unsigned char *)buf, renderWidth, renderHeight);
		img = generateBitmapImageClr((unsigned char *)buf, renderWidth, renderHeight);

		//char pName[MAX_PATH] = "";
		//sprintf(pName, "ass_%06d_%02d.png", i + 1, c);
		//img->Save(Marshal::PtrToStringAnsi((IntPtr)(char *)pName), Imaging::ImageFormat::Png);
	}

	free((void*)buf);

	// TODO: 在此处插入 return 语句
	return img;
}
