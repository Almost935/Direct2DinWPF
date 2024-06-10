using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Direct2DinWPF
{
    class Sample
    {
        //[DllImport(“DxfDrawer.dll”)]
        //internal static extern IntPtr Initialize(IntPtr hwnd, int width, int height);
        //[DllImport(“DxfDrawer.dll”)]
        //internal static extern void Render();
        //[DllImport(“DxfDrawer.dll”)]
        //internal static extern void Cleanup();
    }

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void d3dImage_IsFrontBufferAvailableChanged(object sender, DependencyPropertyChangedEventArgs e)
        {

        }
    }
}