// exception.cpp - implementation of base exceptions.
// Started Summer 2001 by David Megginson, david@megginson.com
// This code is released into the Public Domain.
//

#include "exception.hpp"
#include <stdio.h>

////////////////////////////////////////////////////////////////////////
// Implementation of xh_location class.
////////////////////////////////////////////////////////////////////////

xh_location::xh_location ()
  : _path(""),
	_line(-1),
	_column(-1),
	_byte(-1)
{
}

xh_location::xh_location (const string &path, int line, int column)
  : _path(path),
	_line(line),
	_column(column),
	_byte(-1)
{
}

xh_location::~xh_location ()
{
}

const string &xh_location::getPath () const
{
  return _path;
}

void xh_location::setPath (const string &path)
{
  _path = path;
}

int xh_location::getLine () const
{
  return _line;
}

void xh_location::setLine (int line)
{
  _line = line;
}

int xh_location::getColumn () const
{
  return _column;
}

void xh_location::setColumn (int column)
{
  _column = column;
}

int xh_location::getByte () const
{
  return _byte;
}

void xh_location::setByte (int byte)
{
  _byte = byte;
}

string xh_location::asString () const
{
  char buf[128];
  string out = "";
  if (_path != "") {
	out += _path;
	if (_line != -1 || _column != -1)
	  out += ",\n";
  }
  if (_line != -1) {
	sprintf(buf, "line %d", _line);
	out += buf;
	if (_column != -1)
	  out += ", ";
  }
  if (_column != -1) {
	sprintf(buf, "column %d", _column);
	out += buf;
  }
  return out;
}


////////////////////////////////////////////////////////////////////////
// Implementation of xh_throwable class.
////////////////////////////////////////////////////////////////////////

xh_throwable::xh_throwable ()
  : _message(""),
	_origin("")
{
}

xh_throwable::xh_throwable (const string &message, const string &origin)
  : _message(message),
	_origin(origin)
{
}

xh_throwable::~xh_throwable ()
{
}

const string &xh_throwable::getMessage () const
{
  return _message;
}

const string xh_throwable::getFormattedMessage () const
{
  return getMessage();
}

void xh_throwable::setMessage (const string &message)
{
  _message = message;
}

const string &xh_throwable::getOrigin () const
{
  return _origin;
}

void xh_throwable::setOrigin (const string &origin)
{
  _origin = origin;
}


////////////////////////////////////////////////////////////////////////
// Implementation of xh_error class.
////////////////////////////////////////////////////////////////////////

xh_error::xh_error ()
  : xh_throwable ()
{
}

xh_error::xh_error (const string &message, const string &origin)
  : xh_throwable(message, origin)
{
}

xh_error::~xh_error ()
{
}


////////////////////////////////////////////////////////////////////////
// Implementation of xh_exception class.
////////////////////////////////////////////////////////////////////////

xh_exception::xh_exception ()
  : xh_throwable ()
{
}

xh_exception::xh_exception (const string &message, const string &origin)
  : xh_throwable(message, origin)
{
}

xh_exception::~xh_exception ()
{
}


////////////////////////////////////////////////////////////////////////
// Implementation of xh_io_exception.
////////////////////////////////////////////////////////////////////////

xh_io_exception::xh_io_exception ()
  : xh_exception()
{
}

xh_io_exception::xh_io_exception (const string &message, const string &origin)
  : xh_exception(message, origin)
{
}

xh_io_exception::xh_io_exception (const string &message,
				  const xh_location &location,
				  const string &origin)
  : xh_exception(message, origin),
	_location(location)
{
}

xh_io_exception::~xh_io_exception ()
{
}

const string xh_io_exception::getFormattedMessage () const
{
  string ret = getMessage();
  ret += "\n at ";
  ret += getLocation().asString();
  return ret;
}

const xh_location &xh_io_exception::getLocation () const
{
  return _location;
}

void xh_io_exception::setLocation (const xh_location &location)
{
  _location = location;
}


////////////////////////////////////////////////////////////////////////
// Implementation of xh_format_exception.
////////////////////////////////////////////////////////////////////////

xh_format_exception::xh_format_exception ()
  : xh_exception(),
	_text("")
{
}

xh_format_exception::xh_format_exception (const string &message,
					  const string &text,
					  const string &origin)
  : xh_exception(message, origin),
	_text(text)
{
}

xh_format_exception::~xh_format_exception ()
{
}

const string &xh_format_exception::getText () const
{
  return _text;
}

void xh_format_exception::setText (const string &text)
{
  _text = text;
}


////////////////////////////////////////////////////////////////////////
// Implementation of xh_range_exception.
////////////////////////////////////////////////////////////////////////

xh_range_exception::xh_range_exception ()
  : xh_exception()
{
}

xh_range_exception::xh_range_exception (const string &message,
					const string &origin)
  : xh_exception(message, origin)
{
}

xh_range_exception::~xh_range_exception ()
{
}

