#!/usr/bin/env python2.6
import wx
from wx.glcanvas import GLCanvas
#from wxPython.wx import *
#from wxPython.glcanvas import wxGLCanvas
from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import sys, os

import vtpSimple

name = 'ball_glut'

ID_ABOUT = 101
ID_EXIT  = 102

BUTTON_NUMS = {wx.MOUSE_BTN_LEFT:1, wx.MOUSE_BTN_RIGHT:2, wx.MOUSE_BTN_MIDDLE:3}


class vtGLCanvas(GLCanvas):
    def __init__(self, parent):
        GLCanvas.__init__(self, parent,-1)
        self._captureMouse = False
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_IDLE, self.OnIdle)
        self.Bind(wx.EVT_CHAR, self.OnChar)
        self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        self.Bind(wx.EVT_KEY_UP, self.OnKeyUp)
        self.Bind(wx.EVT_MOUSE_EVENTS, self.OnMouseEvent)
        # this seems to cause a blank rendering
        #wx.EVT_SIZE(self, self.OnSize)
        self.init = 0
        return

    def OnPaint(self,event):
        dc = wx.PaintDC(self)
        self.SetCurrent()
        if not self.init:
            self.InitGL()
            self.init = 1
        
        size = self.GetSize()
        vtpSimple.setCanvasSize(size.width, size.height)

        self.OnDraw()
        return

    def OnDraw(self):
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        vtpSimple.updateScene()
        self.SwapBuffers()
        return

    def OnIdle(self, event):
        self.SetCurrent()
        self.OnDraw()
        return
    
    def OnSize(self, event):
        self.SetCurrent()
        size = event.GetSize()
        vtpSimple.setCanvasSize(size.width, size.height)
        return

    # Keyboard handling
    def OnKeyDown(self, event):
        vtpSimple.setKeyState(event.KeyCode, True)
        return

    def OnKeyUp(self, event):
        vtpSimple.setKeyState(event.KeyCode, False)
        return
    
    def OnChar(self, event):
        vtpSimple.keyEvent(event.KeyCode, 
                           control=event.ControlDown(),
                           shift=event.ShiftDown(),
                           alt=bool(event.AltDown()))
        return

    def OnMouseEvent(self, e):
        kw = {'control': e.ControlDown(),
              'shift': e.ShiftDown(),
              'alt': e.AltDown() }
        
        x,y = e.GetPositionTuple()

        if e.Moving():
            vtpSimple.mouseMoveEvent(x, y, **kw)
            return
        if e.ButtonDown():
            if not self._captureMouse:
                self.CaptureMouse()
                self._captureMouse = True
            vtpSimple.mouseButtonEvent(x, y, BUTTON_NUMS[e.GetButton()], True, **kw)
        elif e.ButtonUp():
            if self._captureMouse:
                self.ReleaseMouse()
                self._captureMouse = False
            vtpSimple.mouseButtonEvent(x, y, BUTTON_NUMS[e.GetButton()], False, **kw)
        return

    def InitGL(self):
        # set viewing projection
        light_diffuse = [1.0, 1.0, 1.0, 1.0]
        light_position = [1.0, 1.0, 1.0, 0.0]

        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse)
        glLightfv(GL_LIGHT0, GL_POSITION, light_position)

        glEnable(GL_LIGHTING)
        glEnable(GL_LIGHT0)
        glEnable(GL_DEPTH_TEST)
        glClearColor(0.0, 0.0, 0.0, 1.0)
        glClearDepth(1.0)

        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(40.0, 1.0, 1.0, 30.0)

        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()
        gluLookAt(0.0, 0.0, 10.0,
                  0.0, 0.0, 0.0,
                  0.0, 1.0, 0.0)
        return
    
class MyFrame(wx.Frame):
    def __init__(self, parent, ID, title, pos, size):
        wx.Frame.__init__(self, parent, ID, title, pos, size)
        self.CreateStatusBar()
        self.SetStatusText("This is the statusbar")

        menu = wx.Menu()
        menu.Append(ID_ABOUT, "&About",
                    "More information about this program")
        menu.AppendSeparator()
        menu.Append(ID_EXIT, "E&xit", "Terminate the program")

        vmenu = wx.Menu()
        vmenu.Append(1001, "Start Interpreter", "Open a Python Interpreter")

        menuBar = wx.MenuBar()
        menuBar.Append(menu, "&File");
        menuBar.Append(vmenu, "&Tools")

        self.Bind(wx.EVT_MENU, self.OpenInterpreter, id=1001)

        self.SetMenuBar(menuBar)
        canvas = vtGLCanvas(self)
        
    def OpenInterpreter(self, event):
        import  wx.py   as  py
        win = py.shell.ShellFrame(self, 0, title="Python Interpeter (Crust)")
        win.Show(True)

    
class MyApp(wx.App):
    def OnInit(self):
        filters = 'XML files (*.xml)|*.xml|All files (*.*)|*.*'
        # get the source file
        dialog = wx.FileDialog ( None, style = wx.OPEN)
        # Show the dialog and get user input
        if dialog.ShowModal() == wx.ID_OK:
            data_file = dialog.GetPath()
        # The user did not select anything
        else:
            data_file = "/data/VTP/Data/Terrains/Honoka'a.xml"
            print 'Nothing was selected. Using default:', data_file
        # Destroy the dialog
        dialog.Destroy()

        data_dir = os.path.join(os.path.dirname(data_file), "..")

        me = MyFrame(None,-1,'vtp_wx_py',wx.DefaultPosition,wx.Size(400,400))
        # hackish for for now.
        vtpSimple.init(data_file, data_dir + "/", "wxPydebug.txt")        
        me.Show()
        return True

    def OnExit(self):
        vtpSimple.shutdown()
        return True
        


def main():
    app = MyApp()
    app.MainLoop()

if __name__ == '__main__': main()
