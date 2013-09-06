using System;
using System.Globalization;
using System.Threading;
using System.Resources;
using System.Reflection;

namespace Horde3DNET.Properties
{
	public static class Resources
	{
		static ResourceManager rm = new ResourceManager("Horde3D_Properties", Assembly.GetExecutingAssembly());
		static CultureInfo ci = Thread.CurrentThread.CurrentCulture;

		public static string LibraryIncompatibleExceptionString {
			get {
				return rm.GetString("LibraryIncompatibleExceptionString", ci);
			}
		}

		public static string LoadResourceArgumentExceptionString {
			get {
				return rm.GetString("LoadResourceArgumentExceptionString", ci);
			}
		}
		
		public static string MatrixOutOfRangeExceptionString {
			get {
				return rm.GetString("MatrixOutOfRangeExceptionString", ci);
			}
		}

		public static string NotImplementedExceptionString {
			get {
				return rm.GetString("NotImplementedExceptionString", ci);
			}
		}

		public static string NotSupportedExceptionString {
			get {
				return rm.GetString("NotSupportedExceptionString", ci);
			}
		}

		public static string StringNullExceptionString {
			get {
				return rm.GetString("StringNullExceptionString", ci);
			}
		}

		public static string UIntOutOfRangeExceptionString {
			get {
				return rm.GetString("UIntOutOfRangeExceptionString", ci);
			}
		}

		public static string VersionString {
			get {
				return rm.GetString("VersionString", ci);
			}
		}

	}
}