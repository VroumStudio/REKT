// InputMgr.mm
// Copyright (C) 2011 Audiokinetic Inc 
/// \file 
/// This class prints coregraphics text to an OpenGL texture
/// This is highly suboptimal so don't use it as is to 
/// display text in your OpenGL game.

#import "TextTexure2D.h"

@implementation TextTexure2D

- (void)dealloc 
{
    glDeleteTextures(1, &m_name);

     //No ARC on Mac Integration Demo project
    #if defined (AK_MAC_OS_X)
    [super dealloc];
    #endif
}

- (id) initWithString:(char*)string withStyle:(DrawStyle) in_eDrawStyle;
{
	NSUInteger i;
	
	CGContextRef			context = NULL;
	void*					data = NULL;
	CGColorSpaceRef			colorSpace = NULL;
	
	m_contentWidth = strlen(string) * GetLineHeight(in_eDrawStyle);
	{
		i = 1;
		while(i < m_contentWidth)
			i *= 2;
		m_textureWidth = i;
	}
	m_contentHeight = GetLineHeight(in_eDrawStyle)*2;
	{
		i = 1;
		while(i < m_contentHeight)
			i *= 2;
		m_textureHeight = i;
	}
	
	char* szFontName;

	// Draw using the font and colour based on the desired DrawStyle
	switch ( in_eDrawStyle )
	{
		case DrawStyle_Title:
			szFontName = (char*)"Helvetica Bold";
			break;
		case DrawStyle_Selected:
			szFontName = (char*)"Helvetica Bold";
			break;
		case DrawStyle_Error:
			szFontName = (char*)"Helvetica";
			break;
		case DrawStyle_Text:
			szFontName = (char*)"Helvetica";
			break;
		case DrawStyle_Control:
			szFontName = (char*)"Helvetica";
			break;
	}
	
	// Using grayscale, color is done by OpenGL
	colorSpace = CGColorSpaceCreateDeviceGray();
	data = calloc(m_textureHeight, m_textureWidth);
	context = CGBitmapContextCreate(data, m_textureWidth, m_textureHeight, 8, m_textureWidth, colorSpace, kCGImageAlphaNone);
	CGColorSpaceRelease(colorSpace);
	
	if(context == NULL) 
	{
		NSLog(@"Failed creating CGBitmapContext");
		free(data);
		return nil;
	}

	CGContextSelectFont (context, szFontName, GetLineHeight(in_eDrawStyle), kCGEncodingMacRoman);
    
	CGContextSetRGBFillColor (context, 1, 1, 1, 1); // 6
    CGContextShowTextAtPoint (context, 0, m_textureHeight - GetLineHeight(in_eDrawStyle), string, strlen(string)); 
	
	GLint saveName;
	glGenTextures(1, &m_name);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &saveName);
	glBindTexture(GL_TEXTURE_2D, m_name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (AkInt32)m_textureWidth, (AkInt32)m_textureHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	
	glBindTexture(GL_TEXTURE_2D, saveName);
	
	if(!CHECK_GL_ERROR()) 
	{
		CGContextRelease(context);
		free(data);
		return nil;
	}

	m_maxS = m_contentWidth / (float)m_textureWidth;
	m_maxT = m_contentHeight / (float)m_textureHeight;
	
	CGContextRelease(context);
	free(data);
	
	return self;
}

- (void) drawAtPoint:(CGPoint)point
{
	GLfloat	coordinates[] = 
	{
		0,	m_maxT,
		m_maxS,	m_maxT,
		0,	0,
		m_maxS,	0
	};
	
	GLfloat	width = (GLfloat)m_textureWidth * m_maxS ,
	height = (GLfloat)m_textureHeight * m_maxT;
	
	GLfloat	vertices[] = 
	{
	static_cast<GLfloat>(point.x),					static_cast<GLfloat>(point.y - height),			-1,
	static_cast<GLfloat>(width + point.x),			static_cast<GLfloat>(point.y - height),			-1,
	static_cast<GLfloat>(point.x),					static_cast<GLfloat>(point.y),	-1,
	static_cast<GLfloat>(width + point.x),			static_cast<GLfloat>(point.y),	-1
	};
	
	glBindTexture(GL_TEXTURE_2D, m_name);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, coordinates);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
@end
