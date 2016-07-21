import wx
    
buttonCount = "red"

frame = None

def report(color):
    """Called by multicastCounter nodes to set new count"""
    global frame
    
    print color
    
    if not frame:
        frame = McastFrame(root)

    frame.displayButtonCount(color)

class McastFrame(wx.Frame):
    """Main window frame"""
    def __init__(self, parent):
        wx.Frame.__init__(self, parent, -1, "Synapse Multicast Counter", style = wx.DEFAULT_FRAME_STYLE | wx.STAY_ON_TOP)
        self.Bind(wx.EVT_CLOSE, self.onClose)
        self.panel = McastPanel(self)
        self.Show(True)

    def onClose(self, event):
        self.Destroy()

    def displayButtonCount(self, newCount):
        #self.panel.sc.SetValue(newCount)
        #self.panel.text.SetLabel('%s' % newCount)
        self.SetBackgroundColour(newCount)
        self.Refresh()
        
        
class McastPanel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, -1)
        self.mcastFrame = parent
        box = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(box)
        
    def OnSpin(self, event):
        global buttonCount
        buttonCount = event.Selection
        self.text.SetLabel('%s' % buttonCount)

if __name__ == '__main__':
    """Mcast Test""" 
    class MyApp(wx.App):
        def OnInit(self):
            self.frame = McastFrame(None)
            self.SetTopWindow(self.frame)
            return True

    app = MyApp(0)
    app.MainLoop()

