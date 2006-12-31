#ifndef _ncbind_hpp_
#define _ncbind_hpp_

#include <windows.h>
#include "tp_stub.h"
#include "ncb_invoke.hpp"


#if (defined(DEBUG) || defined(_DEBUG))
#define NCB_LOG(n)     TVPAddLog(ttstr(n))
#define NCB_LOG_W(log) NCB_LOG(TJS_W(log))
#define NCB_LOG_2(a,b) TVPAddLog(ttstr(a) + ttstr(b))
#else
#define NCB_LOG_VOID   ((void)0)
#define NCB_LOG(n)     NCB_LOG_VOID
#define NCB_LOG_W(log) NCB_LOG_VOID
#define NCB_LOG_2(a,b) NCB_LOG_VOID
#endif
////////////////////////////////////////
/// NativeClass ���O/ID/�N���X�I�u�W�F�N�g�ێ��p
template <class T>
struct ncbClassInfo {
	typedef T NativeClassT;

	typedef tjs_char const*           NameT;
	typedef tjs_int32                 IdentT;
	typedef tTJSNativeClassForPlugin  ObjectT;

	/// �v���p�e�B�擾
	static inline NameT    GetName()        { return _info.name; }
	static inline IdentT   GetID()          { return _info.id; }
	static inline ObjectT *GetClassObject() { return _info.obj; }

	/// �C�j�V�����C�U
	static inline bool Set(NameT name, IdentT id, ObjectT *obj) {
		if (_info.initialized) return false;
		_info.name = name;
		_info.id   = id;
		_info.obj  = obj;
		return (_info.initialized = true);
	}
	/// �ď�����
	static inline void Clear() {
		_info.name = 0;
		_info.id   = 0;
		_info.obj  = 0;
		_info.initialized = false;
	}
private:
	typedef struct info {
		info() : initialized(false), name(0), id(0), obj(0) {}

		bool initialized;
		NameT name;
		IdentT id;
		ObjectT *obj;
	} InfoT;
	static InfoT _info;
};
template <> struct ncbClassInfo<void> {};


////////////////////////////////////////
/// �C���X�^���X�A�_�v�^
template <class T>
struct ncbInstanceAdaptor : public tTJSNativeInstance {
	typedef T NativeClassT;
	typedef ncbInstanceAdaptor<NativeClassT> AdaptorT;
	typedef ncbClassInfo<NativeClassT>       ClassInfoT;

	/*constructor*/ ncbInstanceAdaptor() : _instance(0) {}
	/*destructor*/ ~ncbInstanceAdaptor() { _deleteInstance(); }

	// TJS2 �I�u�W�F�N�g���쐬�����Ƃ��ɌĂ΂��
	//tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj);
	// �c�̂����CTJS_BEGIN_NATIVE_CONSTRUCTOR �}�N������Ă΂��̂�
	// ��L�}�N�����g�p�����ɓƎ��������Ă��邱���ł͎g�p���Ȃ�(��ncbNativeClassConstructor)

	/// �I�u�W�F�N�g�������������Ƃ��ɌĂ΂��
	void TJS_INTF_METHOD Invalidate() { _deleteInstance(); }

private:
	/// ���C���X�^���X�ւ̃|�C���^
	NativeClassT *_instance;

	/// ���C���X�^���X�j��
	void _deleteInstance() {
		if (_instance) delete _instance;
		_instance = 0;
	}

	//--------------------------------------
	// static�w���p�֐�
public:

	/// iTJSDispatch2 ���� Adaptor ���擾
	static AdaptorT *GetAdaptor(iTJSDispatch2 *obj) {
		iTJSNativeInstance* adp = 0;
		if (!obj) {
			TVPThrowExceptionMessage(TJS_W("No instance."));
			return 0;
		}
		if (TJS_FAILED(obj->NativeInstanceSupport(TJS_NIS_GETINSTANCE, ClassInfoT::GetID(), &adp))) {
			TVPThrowExceptionMessage(TJS_W("Invalid instance type."));
			return 0;
		}
		return static_cast<AdaptorT*>(adp);
	}

	/// iTJSDispatch2 ���� NativeClass�C���X�^���X���擾
	static NativeClassT *GetNativeInstance(iTJSDispatch2 *obj) {
		AdaptorT *adp = GetAdaptor(obj);
		return adp ? adp->_instance : 0;
	}

	/// NativeClass�C���X�^���X��ݒ�
	static bool SetNativeInstance(iTJSDispatch2 *obj, NativeClassT *instance) {
		AdaptorT *adp = GetAdaptor(obj);
		if (!adp) return false;
		adp->_instance = instance;
		return true;
	}

	/// �N���X�I�u�W�F�N�g����Adaptor�C���X�^���X�𐶐�����instance����
	static iTJSDispatch2* CreateAdaptor(NativeClassT *inst) {
		typename ClassInfoT::ObjectT *clsobj = ClassInfoT::GetClassObject();
		if (!clsobj) {
			TVPThrowExceptionMessage(TJS_W("No class object."));
			return 0;
		}

		iTJSDispatch2 *global = TVPGetScriptDispatch(), *obj = 0;
		tTJSVariant dummy, *param = &dummy;
		// ������1�ł���void�ł���Ύ��C���X�^���X��new���Ȃ�����ɂȂ�
		tjs_error r = clsobj->CreateNew(0, NULL, NULL, &obj, 1, &param, global);
		if (global) global->Release();

		if (TJS_FAILED(r) || !obj) {
			TVPThrowExceptionMessage(TJS_W("Can't create instance"));
			return 0;
		}
		AdaptorT *adp = GetAdaptor(obj);
		if (adp) adp->_instance = inst;
		return obj;
	}

	/// ��� Adaptor �𐶐����� (tTJSNativeClassForPlugin�^�̊֐�)
	static iTJSNativeInstance* TJS_INTF_METHOD CreateEmptyAdaptor() {
		return static_cast<iTJSNativeInstance*>(new AdaptorT());
	}
};

////////////////////////////////////////
/// �^�ϊ��p�w���p�e���v���[�g
struct ncbTypeConvertor {

	template <typename FROM, typename TO>
	struct Conversion {
	private:
		typedef char OK;
		typedef struct { char ng[2]; } NG;
		static OK check(TO);
		static NG check(...);
		static FROM wrap();
	public:
		enum {
			Exists = (sizeof(check(wrap())) == sizeof(OK)),
			Same   = false
		};
	};
	template <typename T> struct Conversion<T, T>       { enum { Exists = true,  Same = true  }; };
	template <typename T> struct Conversion<T, void>    { enum { Exists = false, Same = false }; };
	template <typename T> struct Conversion<void, T>    { enum { Exists = false, Same = false }; };
//	template <>           struct Conversion<void, void> { enum { Exists = false, Same = true  }; };
#define NCB_INNER_CONVERSION_SPECIALIZATION \
	template <> struct ncbTypeConvertor::Conversion<void, void> { enum { Exists = false, Same = true  }; };

	/// �C���q���O��(�|�C���^�C�Q�ƁCconst�����O�����f�̌^�� typedef �����)
	template <typename T> struct Stripper             { typedef T Type; };
	template <typename T> struct Stripper<T*>         { typedef typename Stripper<T>::Type Type; };
	template <typename T> struct Stripper<T&>         { typedef typename Stripper<T>::Type Type; };
	template <typename T> struct Stripper<const    T> { typedef typename Stripper<T>::Type Type; };
//	template <typename T> struct Stripper<volatile T> { typedef typename Stripper<T>::Type Type; };

	/// �|�C���^�擾
	template <typename T> struct ToPointer            { static T* Get(T &t) { return &t; } };
	template <typename T> struct ToPointer<T&>        { static T* Get(T &t) { return &t; } };
	template <typename T> struct ToPointer<T*>        { static T* Get(T* t) { return  t; } };
	template <typename T> struct ToPointer<T const&>  { static T* Get(T const &t) { return const_cast<T*>(&t); } };
	template <typename T> struct ToPointer<T const*>  { static T* Get(T const *t) { return const_cast<T*>( t); } };

	/// �|�C���^�ˏC���q�ϊ�
	template <typename T> struct ToTarget             { static T  Get(T *t) { return T(*t); } };
	template <typename T> struct ToTarget<T&>         { static T& Get(T *t) { return   *t; } };
	template <typename T> struct ToTarget<T*>         { static T* Get(T *t) { return    t; } };


	// ���R�s�[����
	struct DirectCopy {
		template <typename DST, typename SRC>
		inline void operator()(DST &dst, SRC const &src) const { dst = src; }
	};

	// �L���X�g����
	template <typename CAST>
	struct CastCopy {
		template <typename DST, typename SRC>
		inline void operator()(DST &dst, SRC const &src) const { dst = static_cast<DST>(static_cast<CAST>(src)); }
	};

	// �^���ꉻ�����݂��邩�}�b�v
	template <typename T, bool SRCF>
	struct SpecialMap {
		enum { Exists = false, Modifier = false, IsSource = SRCF };
		typedef T Type;
	};

	/// ���얢��i�R���p�C���G���[�p�j
	struct NoImplement;

	// �R���o�[�^����I��
	struct SelectConvertorTypeBase {
	protected:
		/// �R�����Z�q
		template <bool EXP, class THEN, class ELSE> struct ifelse                   { typedef ELSE Type; };
		template <          class THEN, class ELSE> struct ifelse<true, THEN, ELSE> { typedef THEN Type; };

		template <typename T, bool IsSrcF>
		struct hasSpecial {
			typedef typename Stripper<T>::Type StripT;
			typedef SpecialMap<T,      IsSrcF> ThisMapT;
			typedef SpecialMap<StripT, IsSrcF> StripMapT;
			enum {
				exthis  = ThisMapT::Exists,
				exstrip = StripMapT::Exists && StripMapT::Modifier,
				Exists = exthis | exstrip,
			};
			typedef typename ifelse<exthis,  typename ThisMapT::Type,
			/*    */typename ifelse<exstrip, typename StripMapT::Type, void>::Type
				/*                */>::Type Type;
		};
		template <typename T> struct wrap { typedef T Type; };

		template <typename SRC, typename DST>
		struct directSelect {
			typedef typename ifelse<Conversion<SRC, DST>::Exists, DirectCopy, NoImplement>::Type Type;
		};
	};
	template <typename SRC, typename DST>
	struct SelectConvertorType : public SelectConvertorTypeBase {
	private:
		typedef hasSpecial<SRC, true > SrcSpecialT;
		typedef hasSpecial<DST, false> DstSpecialT;

		struct specialSelect {
			typedef typename ifelse<DstSpecialT::Exists, typename DstSpecialT::Type, typename SrcSpecialT::Type>::Type Type;
		};
		typedef typename ifelse<
			SrcSpecialT::Exists || DstSpecialT::Exists, specialSelect, directSelect<SRC, DST>
				>::Type select;
	public:
		typedef typename select::Type Type;
	};
};
       NCB_INNER_CONVERSION_SPECIALIZATION
#undef NCB_INNER_CONVERSION_SPECIALIZATION

#define NCB_TYPECONV_MAPSET(mapsel, type, conv, mod) \
	template <> struct ncbTypeConvertor::SpecialMap<type, mapsel> { \
		enum { Exists = true, Modifier = mod, IsSource = mapsel }; \
		typedef conv Type; \
	}

#define NCB_TYPECONV_SRCMAP_SET(type, conv, mod) NCB_TYPECONV_MAPSET(true,  type, conv, mod)
#define NCB_TYPECONV_DSTMAP_SET(type, conv, mod) NCB_TYPECONV_MAPSET(false, type, conv, mod)

#define NCB_TYPECONV_CAST(type, cast) \
	NCB_TYPECONV_SRCMAP_SET(type, ncbTypeConvertor::CastCopy<cast>, false); \
	NCB_TYPECONV_DSTMAP_SET(type, ncbTypeConvertor::CastCopy<cast>, false)

#define NCB_TYPECONV_CAST_INTEGER(type)    NCB_TYPECONV_CAST(type, tTVInteger)
#define NCB_TYPECONV_CAST_REAL(type)       NCB_TYPECONV_CAST(type, tTVReal)

/// ���l�̓L���X�g�ŕϊ�����
NCB_TYPECONV_CAST_INTEGER(   signed char);
NCB_TYPECONV_CAST_INTEGER( unsigned char);
NCB_TYPECONV_CAST_INTEGER(    signed int);
NCB_TYPECONV_CAST_INTEGER(  unsigned int);
NCB_TYPECONV_CAST_INTEGER(  signed short);
NCB_TYPECONV_CAST_INTEGER(unsigned short);
NCB_TYPECONV_CAST_INTEGER(   signed long);
NCB_TYPECONV_CAST_INTEGER( unsigned long);
NCB_TYPECONV_CAST_REAL(            float);
NCB_TYPECONV_CAST_REAL(           double);
NCB_TYPECONV_CAST(            bool, bool);

/// �ꎞ�I�Ƀo�b�t�@���m�ۂ��Ă����� NarrowStr �Ƃ��ď�������
struct ncbVariatToNChar {
	ncbVariatToNChar() : _nstr(0) {}
	~ncbVariatToNChar() {
		if (_nstr) {
			NCB_LOG_W("~ncbVariatToNChar > delete[]");
			delete[] _nstr;
		}
	}
	template <typename DST>
	inline void operator()(DST &dst, tTJSVariant const &src) {
		// �Ȃ� tTJSVariant::Type() �� const ���\�b�h�łȂ��̂��c
		if ((const_cast<tTJSVariant*>(&src))->Type() == tvtString) {
			tTJSString s(src.AsStringNoAddRef());
			tjs_int len = s.GetNarrowStrLen();

			NCB_LOG_W("ncbVariatToNChar::operator() > new tjs_nchar[]");
			_nstr = new tjs_nchar[len+1];
			s.ToNarrowStr(_nstr, len+1);
		}
		dst = static_cast<DST>(_nstr);
	}
private:
	tjs_nchar *_nstr;
};

// Narrow������Ƃ��ēo�^����}�N��
#define NCB_TYPECONV_NARROW_STRING(type) \
	NCB_TYPECONV_SRCMAP_SET(type, ncbTypeConvertor::CastCopy<tjs_nchar const*>, false); \
	NCB_TYPECONV_DSTMAP_SET(type, ncbVariatToNChar, false)

/// signed char �� char ���ĕʕ��Ȃ̂�����H
NCB_TYPECONV_NARROW_STRING(         char const*);
NCB_TYPECONV_NARROW_STRING(  signed char const*);
NCB_TYPECONV_NARROW_STRING(unsigned char const*);



/// Boxing/Unboxing
struct ncbNativeObjectBoxing {
	typedef tTJSVariant VarT;
	typedef ncbTypeConvertor ConvT;
	/// Boxing
	struct Boxing { enum { HasWork = false };
		template <typename SRC>
		inline void operator ()(VarT &dst, SRC const &src) const {
			typedef SRC                                     TargetT;
			typedef typename ConvT::Stripper<TargetT>::Type ClassT;
			typedef ncbInstanceAdaptor<ClassT>              AdaptorT;

			ClassT *p = ConvT::ToPointer<TargetT>::Get(src);		//< ���C���X�^���X�|�C���^
			iTJSDispatch2 *adpobj = AdaptorT::CreateAdaptor(p);		//< �A�_�v�^TJS�I�u�W�F�N�g����
			dst = adpobj;											//< Variant�ɃR�s�[
			adpobj->Release();										//< �R�s�[�ς݂Ȃ̂�adaptor�͕s�v
		}
	};

	/// Unboxing
	struct Unboxing { enum { HasWork = false };
		template <typename DST>
		inline void operator ()(DST &dst, VarT const &src) const {
			typedef DST                                     TargetT;
			typedef typename ConvT::Stripper<TargetT>::Type ClassT;
			typedef ncbInstanceAdaptor<ClassT>              AdaptorT;

			iTJSDispatch2 *obj = src.AsObjectNoAddRef();			//< �Q�ƃJ�E���^�����Ȃ���Dispatch�I�u�W�F�N�g�擾
			ClassT *p = AdaptorT::GetNativeInstance(obj);			//< ���C���X�^���X�̃|�C���^���擾
			dst = ConvT::ToTarget<TargetT>::Get(p);					//< �K�v�Ƃ����^�ɕϊ����ĕԂ�
		}
	};
};

#define NCB_TYPECONV_BOXING(type) \
	NCB_TYPECONV_SRCMAP_SET(type, ncbNativeObjectBoxing::Boxing,   true); \
	NCB_TYPECONV_DSTMAP_SET(type, ncbNativeObjectBoxing::Unboxing, true)



/// ���ꉻ�p�̃}�N�� (�����ϊ��łȂ����Ŏw�肷��ꍇ�͂��̃}�N�����g�p����)
#define NCB_SET_TOVARIANT_CONVERTOR(type, convertor) \
	template <> struct ncbTypeConvertor::SelectConvertorType<type, tTJSVariant> { typedef convertor Type; }

#define NCB_SET_TOVALUE_CONVERTOR(type, convertor) \
	template <> struct ncbTypeConvertor::SelectConvertorType<tTJSVariant, type> { typedef convertor Type; }

#define NCB_SET_CONVERTOR(type, convertor) \
	NCB_SET_TOVARIANT_CONVERTOR(type, convertor); \
	NCB_SET_TOVALUE_CONVERTOR(  type, convertor) \

/// �Ԃ�l�Ȃ��̏ꍇ�̃_�~�[�� TOVARIANT ��o�^
NCB_SET_TOVARIANT_CONVERTOR(void, struct {});


/*
	�^�ϊ��𒼂ŏ�����������

	struct CustomConvertor {
		void operator ()(tTJSVariant &dst, CustomType const &src);
		void operator ()(CustomType const &src, tTJSVariant &dst);
	};
	NCB_SET_CONVERTOR(CustomType, CustomConvertor);

	�Ƃ������悤�Ȋ����œK����
 */



////////////////////////////////////////
/// ���\�b�h�Ăяo���p�x�[�X�N���X
struct ncbNativeClassMethodBase : public tTJSDispatch {
	typedef tTJSDispatch    BaseT;
	typedef const tjs_char *NameT;
	typedef tjs_uint32      FlagsT;

	/// constructor
	ncbNativeClassMethodBase(NameT n) : _name(n) {} //< �N���X����_name�ɕێ� (Function or Property)
	~ncbNativeClassMethodBase() {}

	/// IsInstanceOf ����
	tjs_error TJS_INTF_METHOD IsInstanceOf(
		tjs_uint32 flag, const tjs_char *membername, tjs_uint32 *hint, 
		const tjs_char *classname, iTJSDispatch2 *objthis)
	{
		// �������g(membername==0)�Ŕ�r�N���X����_name�Ȃ�TRUE�C����ȊO�͊ۓ���
		return (!membername && !TJS_stricmp(classname, _name)) ? TJS_S_TRUE
			: BaseT::IsInstanceOf(flag, membername, hint, classname, objthis);
	}

	/// ��̃��\�b�h(finalize Callback�p) �����ɒu���Ă悢�̂���������
	static tjs_error TJS_INTF_METHOD EmptyCallback(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) { return TJS_S_OK; }

private:
	NameT _name;

	// private�ŉB�����Ă݂�
	typedef MethodCaller    CallerT;


	//--------------------------------------
protected:
	/// tMethodTraits���b�p�[
	template <typename T>
	struct traits {
		typedef CallerT::tMethodTraits<T> TraitsT;
		typedef typename TraitsT::ResultType ResultT;
		typedef typename TraitsT::ClassType ClassT;
		typedef typename TraitsT::ArgsType  ArgsT;
		enum { ArgsCount = TraitsT::ArgsCount };
	};


	//--------------------------------------
public:
	/// TJSNativeClassRegisterNCM �ɓn���t���O
	virtual FlagsT GetFlags() const { return 0; }
	//// �ȉ��C�^����t���O�����肷�邽�߂̃w���p�e���v���[�g�Q
private:
	template <typename T> struct flagsSelector       { enum { Flags = 0 }; };
#define NCB_INNER_FLAGSELECTOR_SPECIALIZATION \
	template <>           struct ncbNativeClassMethodBase::flagsSelector<void> { enum { Flags = TJS_STATICMEMBER }; };

protected:
	template <typename T> 
	struct flags { enum { Flags = flagsSelector<typename traits<T>::ClassT>::Flags }; };


	//--------------------------------------
private:

	/// ����/�Ԃ�l�̈����n���p�t�@���N�^ (ncb_invoke.hpp/MethodCaller�ɓn�����߂ɕK�v)
	template <typename RES, class ARGS, int ARGC>
	struct paramsFunctor {
		typedef tTJSVariant      VariantT;
		typedef ncbTypeConvertor ConvT;
		typedef typename     ConvT::SelectConvertorType<RES, VariantT>::Type ResultConvT;
		template <typename T> struct ArgsExtor { typedef typename ConvT::SelectConvertorType<VariantT, T>::Type Type; };
		typedef CallerT::tArgsExtract<ArgsExtor, ARGS, ARGC> ArgsConvT;
		/* ArgsConvT �͈ȉ��̂悤�ɓW�J�����F
			struct ArgsConvT {
				ncbToValueConvertor<ARGS::Arg1Type> t1; // ��Ԗڂ̈����� ncbToValueConvertor
				ncbToValueConvertor<ARGS::Arg2Type> t2; // ��Ԗڂ�
				  :
				ncbToValueConvertor<ARGS::Arg[ARGC]Type> tARGC; // ARGC�Ԗڂ�
			}; */

		/// constructor
		paramsFunctor(VariantT *r, tjs_int n, VariantT const *const *p) : _result(r), _numparams(n), _param(p) {}

		/// ������ NativeClassMethod �ֈ��n��
		template <int N, typename T>
		inline T operator ()(CallerT::tNumTag<N> const &index, CallerT::tTypeTag<T> const &type) {
			T ret;
			// N�Ԗڂ� ncbToValueConvertor �����o���ĕϊ�
			(CallerT::tArgsSelect<ArgsConvT, N>::Get(_aconv))(ret, (_numparams >= N) ? *(_param[N-1]) : VariantT());
			return ret;
		}

		/// NativeClassMethod �̕Ԃ�l��result�֊i�[
		template <typename ResultT>
		inline void operator = (ResultT r) {
			if (_result) _rconv(*_result, r);
		}
	private:
		ArgsConvT      _aconv;
		ResultConvT    _rconv;

		tjs_int        _numparams;
		VariantT *  _result;
		VariantT const *const * _param;
	};
	template <typename METHOD>
	struct functor {
		typedef paramsFunctor<
			typename traits<METHOD>::ResultT,
		/**/typename traits<METHOD>::ArgsT,
		/**/traits<METHOD>::ArgsCount> FunctorT;
	};

	//--------------------------------------
protected:
	/// �^�������n�����߂̃^�O
	template <typename T> struct factoryTag {};

	/// �R���X�g���N�^���C������
	template <typename METHOD>
	static inline tjs_error Factory(factoryTag<METHOD> const&, iTJSDispatch2 *objthis, tTJSVariant *result, tjs_int numparams, tTJSVariant const *const *param) {
		typedef typename traits<METHOD>::ClassT ClassT;
		typedef ncbInstanceAdaptor<ClassT> AdaptorT;

		if (result) result->Clear();

		// ���������ЂƂł���void�̏ꍇ�̓C���X�^���X��ݒ肵�Ȃ�
		if (numparams == 1 && const_cast<tTJSVariant*>(param[0])->Type() == tvtVoid) {
			NCB_LOG_W("Constructor(void)");
			return TJS_S_OK;
		}
		// �������̌������Ȃ��ꍇ�̓G���[
		if (numparams < traits<METHOD>::ArgsCount)
			return TJS_E_BADPARAMCOUNT;

		ClassT* instance;
		{
			// �����󂯓n���t�@���N�^
			typename functor<METHOD>::FunctorT fnct(result, numparams, param);
			// MethodCaller::Factory �Ăяo��(ClassT�̃C���X�^���X�쐬)
			instance = CallerT::Factory(fnct, CallerT::tTypeTag<METHOD>());
			// paramsFunctor���ł�����
		}
		if (!instance) {
			TVPThrowExceptionMessage(TJS_W("NativeClassInstance creation faild."));
			return TJS_E_FAIL;
		}

		// �G���[�������ǂ��Ȃ�̂��悭�킩��񂯂ǓK���Ɂi��
		tjs_error r = TJS_S_OK;
		try {
			if (!AdaptorT::SetNativeInstance(objthis, instance)) {
				r = TJS_E_NATIVECLASSCRASH;
				delete instance;
			}
		} catch (...) {
			if (instance) delete instance;
			throw;
		}
		return r;
	}
	//--------------------------------------
private:
	/// static/��static�̏ꍇ�킯Invoke�̂��߂̃e���v���[�g
	template <class T>
	struct invokeImpl {
		/// ��static�̏ꍇ
		template <typename METHOD>
		static inline tjs_error Invoke(METHOD const &m, iTJSDispatch2 *objthis, tTJSVariant *result, tjs_int numparams, tTJSVariant const *const *param) {
			typedef T  ClassT;
			typedef ncbInstanceAdaptor<ClassT> AdaptorT;
			typedef typename functor<METHOD>::FunctorT FunctorT;

			// ���C���X�^���X�̃|�C���^
			ClassT *obj = AdaptorT::GetNativeInstance(objthis);
			if (!obj) return TJS_E_NATIVECLASSCRASH;

			// �C���X�^���X��n���ăN���X���\�b�h�����s
			FunctorT fnct(result, numparams, param);
			return CallerT::Invoke(fnct, m, obj) ? TJS_S_OK : TJS_E_FAIL;
		}
	};
protected:
	/// ���\�b�h�Ăяo�������
	template <typename METHOD>
	static inline tjs_error Invoke(METHOD const &m, iTJSDispatch2 *objthis, tTJSVariant *result, tjs_int numparams, tTJSVariant const *const *param) {
		// �������̌������Ȃ��ꍇ�̓G���[
		if (numparams < traits<METHOD>::ArgsCount)
			return TJS_E_BADPARAMCOUNT;

		if (result) result->Clear();

		// MethodCaller::Invoke �Ăяo��(static/��static �ŏꍇ�킯)
		return invokeImpl<typename traits<METHOD>::ClassT>::Invoke(m, objthis, result, numparams, param);
	}
};
/// invokeImpl ���ꉻ�iClassType �� void �Ȃ� static ���\�b�h (MethodCaller�d�l)
template <>
struct ncbNativeClassMethodBase::invokeImpl<void> {
	/// static���\�b�h��Invoke����ꍇ
	template <typename METHOD>
	static tjs_error Invoke(METHOD const &m, iTJSDispatch2 *objthis, tTJSVariant *result, tjs_int numparams, tTJSVariant const *const *param) {
		typename functor<METHOD>::FunctorT fnct(result, numparams, param);
		return CallerT::Invoke(fnct, m) ? TJS_S_OK : TJS_E_FAIL; // ���Ăт�OK
	}
};

/// ncbNativeClassMethodBase::flagsSelector���ꉻ(���ꉻ�̓N���X��`���ɂ͋L�q�ł��Ȃ��炵��)
       NCB_INNER_FLAGSELECTOR_SPECIALIZATION
#undef NCB_INNER_FLAGSELECTOR_SPECIALIZATION



////////////////////////////////////////
/// ���\�b�h�Ăяo���N���X�e���v���[�g
// �{���� TJSCreateNativeClassMethod�i�y�ыg���g������tTJSNativeClassMethod�j���g�p����Ƃ����
// ���O�Ŏ�������(TJSCreateNativeClassMethod�ł�static�Ȋ֐������ĂׂȂ��̂Ń��\�b�h�ւ̃|�C���^�̕ێ�������Ȃ���)
template <typename METHOD>
struct ncbNativeClassMethod : public ncbNativeClassMethodBase { 
	typedef METHOD MethodT;

	/// constructor
	ncbNativeClassMethod(MethodT m)
		: ncbNativeClassMethodBase(TJS_W("Function")), // TJS�I�u�W�F�N�g�I�ɂ� Function
		  _method(m)
	{
		if (!_method) TVPThrowExceptionMessage(TJS_W("No method pointer."));
	} 

	/// FuncCall����
	tjs_error  TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint, 
		tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		// �������g���Ă΂ꂽ�̂ł͂Ȃ��ꍇ�͊ۓ���
		if (membername) return BaseT::FuncCall(flag, membername, hint, result, numparams, param, objthis);

		// �G���[�`�F�b�N
		if (!objthis) return TJS_E_NATIVECLASSCRASH;

		// ���\�b�h�Ăяo��
		return Invoke(_method, objthis, result, numparams, param);
	}
	/// TJSNativeClassRegisterNCM�t���O
	FlagsT GetFlags() const { return flags<MethodT>::Flags; }

private:
	MethodT const _method;
};

////////////////////////////////////////
/// �R���X�g���N�^�Ăяo���N���X�e���v���[�g
template <typename METHOD>
struct ncbNativeClassConstructor : public ncbNativeClassMethodBase {
	typedef METHOD MethodT;

	/// constructor
	ncbNativeClassConstructor()
		: ncbNativeClassMethodBase(TJS_W("Function")) {} // TJS�I�u�W�F�N�g�I�ɂ� Function

	/// FuncCall����
	tjs_error  TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint, 
		tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		// �������g���Ă΂ꂽ�̂ł͂Ȃ��ꍇ�͊ۓ���
		if (membername) return BaseT::FuncCall(flag, membername, hint, result, numparams, param, objthis);

		// �R���X�g���N�^�Ăяo��
		return Factory(factoryTag<MethodT>(), objthis, result, numparams, param); 
	}

	/// TJSNativeClassRegisterNCM�t���O
//	FlagsT GetFlags() const { return 0; }
}; 


////////////////////////////////////////

template <typename GETTER, typename SETTER>
struct ncbNativeClassProperty : public ncbNativeClassMethodBase {
	typedef GETTER GetterT;
	typedef SETTER SetterT;

	/// constructor
	ncbNativeClassProperty(GetterT get, SetterT set)
		: ncbNativeClassMethodBase(TJS_W("Property")), // TJS�I�u�W�F�N�g�I�ɂ� Property
		  _getter(get), _setter(set) {} 

	/// PropGet ����
	tjs_error TJS_INTF_METHOD PropGet(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint, 
		tTJSVariant *result, iTJSDispatch2 *objthis)
	{
		// �������g���Ă΂ꂽ�̂ł͂Ȃ��ꍇ�͊ۓ���
		if (membername) return BaseT::PropGet(flag, membername, hint, result, objthis);

		// �G���[�`�F�b�N
		if (!_getter) return TJS_E_ACCESSDENYED;
		if (!objthis) return TJS_E_NATIVECLASSCRASH;

		// ���\�b�h�Ăяo��
		return Invoke(_getter, objthis, result, 0, 0);
	}

	/// PropSet ����
	tjs_error TJS_INTF_METHOD PropSet(
		tjs_uint32 flag, const tjs_char *membername, tjs_uint32 *hint, 
		const tTJSVariant *param, iTJSDispatch2 *objthis)
	{
		// �������g���Ă΂ꂽ�̂ł͂Ȃ��ꍇ�͊ۓ���
		if (membername) return BaseT::PropSet(flag, membername, hint, param, objthis);

		// �G���[�`�F�b�N
		if (!_setter) return TJS_E_ACCESSDENYED;
		if (!objthis) return TJS_E_NATIVECLASSCRASH;
		if (!param)   return TJS_E_FAIL;

		// ���\�b�h�Ăяo��
		return Invoke(_setter, objthis, 0, 1, &param);
	}

	/// TJSNativeClassRegisterNCM�t���O
	FlagsT GetFlags() const { return flags<GetterT>::Flags; }

private:
	/// Getter �� Setter �������N���X�̃��\�b�h���ǂ����𔻒�i�Ⴄ�ꍇ�̓R���p�C���G���[�j
	template <class T1, class T2> struct sameClassChecker;
	template <class T>            struct sameClassChecker<T, T> { enum { OK }; };
	enum { ClassCheck = sameClassChecker<
		typename traits<GetterT>::ClassT,
		typename traits<SetterT>::ClassT >::OK
	};

	/// �v���p�e�B���\�b�h�ւ̃|�C���^
	GetterT const _getter;
	SetterT const _setter;
};


////////////////////////////////////////
/// ���R�[���o�b�N�p
template <class T>
struct ncbRawCallbackMethod : public ncbNativeClassMethodBase {
	typedef T NativeClassT;
	typedef ncbRawCallbackMethod<NativeClassT> ThisClassT;
	typedef ncbInstanceAdaptor<NativeClassT>     AdaptorT;

	typedef tjs_error (TJS_INTF_METHOD *CallbackT)(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, NativeClassT *nativeInstance);

	/// constructor
	ncbRawCallbackMethod(CallbackT m, FlagsT f)
		: ncbNativeClassMethodBase(TJS_W("Function")), // TJS�I�u�W�F�N�g�I�ɂ� Function
		  _callback(m), _flag(f)
	{
		if (!_callback) TVPThrowExceptionMessage(TJS_W("No callback pointer."));
	}

	/// FuncCall����
	tjs_error  TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint, 
		tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		// �������g���Ă΂ꂽ�̂ł͂Ȃ��ꍇ�͊ۓ���
		if (membername) return BaseT::FuncCall(flag, membername, hint, result, numparams, param, objthis);

		// �G���[�`�F�b�N
		if (!objthis) return TJS_E_NATIVECLASSCRASH;

		// �Ԃ�l�N���A
		if (result) result->Clear();

		NativeClassT *obj = 0;
		if (!(_flag & TJS_STATICMEMBER)) {
			//< ���C���X�^���X�̃|�C���^
			obj = AdaptorT::GetNativeInstance(objthis); 
			if (!obj) return TJS_E_NATIVECLASSCRASH;
		}
		// Callback�Ăяo��
		return _callback(result, numparams, param, obj);     //< Callback�Ăяo��
	}

	static ThisClassT *CreateObject(CallbackT cb, FlagsT flag) {
		return new ThisClassT(cb, flag);
	}
private:
	CallbackT const _callback;
	FlagsT    const _flag;
};

template <>
struct ncbRawCallbackMethod<void> {
	typedef tTJSNativeClassMethodCallback CallbackT;
	typedef tjs_uint32                     FlagsT;
	/// 
	static tTJSNativeClassMethod* CreateObject(CallbackT cb, FlagsT flag) {
		// �g���g���]���̌Ăяo��
		return TJSCreateNativeClassMethod(cb);
	}
};



////////////////////////////////////////
/// NativeClass �N���X�I�u�W�F�N�g����
template <class T>
struct ncbCreateClass {
//	typedef typename ncbModiferStripper<T>::Type NativeClassT;
	typedef T                                    NativeClassT;
	typedef ncbCreateClass<NativeClassT>         ThisClassT;

	typedef ncbClassInfo<NativeClassT>           ClassInfoT;
	typedef typename ClassInfoT::IdentT          IdentT;
	typedef typename ClassInfoT::NameT           NameT;
	typedef typename ClassInfoT::ObjectT         ClassObjectT;
	typedef ncbInstanceAdaptor<NativeClassT>     AdaptorT;
	typedef MethodCaller                         CallerT;

	/// �R���X�g���N�^�����̎󂯓n���Ŏg�p
	template <class ARG> struct ConstructorArgs {};

	/// constructor
	ncbCreateClass(NameT name)
		: _hasCtor(false),
		  _classname(name), // �N���X����ێ�
		  _classobj(TJSCreateNativeClassForPlugin(name, AdaptorT::CreateEmptyAdaptor)), // �N���X�I�u�W�F�N�g�𐶐�(ID�������O)
		  _classid(TJSRegisterNativeClass(name)) // �N���XID�𐶐�
	{
		TVPAddLog(ttstr(TJS_W("BeginRegistClass: ")) + ttstr(_classname));

		// ncbClassInfo�ɓo�^
		if (!ClassInfoT::Set(_classname, _classid, _classobj)) {
			TVPThrowExceptionMessage(TJS_W("Already registerd class."));
		}

		// �N���X�I�u�W�F�N�g��ID��ݒ�
		TJSNativeClassSetClassID(_classobj, _classid);

		// ���finalize���\�b�h��ǉ�
		TJSNativeClassRegisterNCM(_classobj, TJS_W("finalize"),
								  TJSCreateNativeClassMethod(ncbNativeClassMethodBase::EmptyCallback),
								  _classname, nitMethod);
	}

	/// destructor
	~ncbCreateClass() {
		// TJS �̃O���[�o���I�u�W�F�N�g���擾����
		iTJSDispatch2 *global = TVPGetScriptDispatch();
		if (!global) return;

		// 2 _classobj �� tTJSVariant �^�ɕϊ�
		tTJSVariant val(static_cast<iTJSDispatch2*>(_classobj));

		// 3 ���ł� val �� _classobj ��ێ����Ă���̂ŁA_classobj �� Release ����
		//_classobj->Release();
		// �c�Ƃ������Ƃ��낾���CBoxing�����ŃC���X�^���X�����֌W�ŃN���X�I�u�W�F�N�g���K�v�Ȃ̂ŊJ�����Ȃ�
		// (�v���O�C���J�����Ƀ����[�X�����ncbCreateClass::Release)

		// 4 global �� PropSet ���\�b�h��p���A�I�u�W�F�N�g��o�^����
		global->PropSet(
			TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
			_classname, // �����o��
			NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
			&val, // �o�^����l
			global // �R���e�L�X�g ( global �ł悢 )
			);

		// - global �� Release ����
		global->Release();

		// val ���N���A����B
		// ����͕K���s���B�������Ȃ��� val ���ێ����Ă���I�u�W�F�N�g
		// �� Release ���ꂸ�A���Ɏg�� TVPPluginGlobalRefCount �����m�ɂȂ�Ȃ��B
		//val.Clear();
		// �c�̂������[�J���X�R�[�v�Ŏ����I�ɏ��������̂ŕK�v�Ȃ�
		TVPAddLog(ttstr(TJS_W("EndRegistClass: ")) + ttstr(_classname));
	}

	/// �v���O�C���J�����ɃN���X�I�u�W�F�N�g�������[�X���邽�߂�static�֐�
	static void Release() {
		iTJSDispatch2 *global = TVPGetScriptDispatch();
		ClassObjectT *obj = ClassInfoT::GetClassObject();
		NameT name = ClassInfoT::GetName();

		if (global && name) global->DeleteMember(0, name, 0, global);
		if (obj) obj->Release();
		if (global) global->Release();

		ClassInfoT::Clear();
	}

	/// ���\�b�h��o�^����
	template <typename MethodT>
	ThisClassT &Method(NameT name, MethodT m) {
		typedef ncbNativeClassMethod<MethodT> MethodObjectT;
		MethodObjectT *mobj = new MethodObjectT(m);
		TJSNativeClassRegisterNCM(_classobj, name, mobj, _classname, nitMethod, mobj->GetFlags());
		return *this;
	}

	/// �R���X�g���N�^��o�^����
	template <typename MethodT>
	ThisClassT &Constructor(ConstructorArgs<MethodT>) {
		typedef ncbNativeClassConstructor<MethodT> CtorObjectT;
		if (!_hasCtor) {
			CtorObjectT *ctor = new CtorObjectT();
			TJSNativeClassRegisterNCM(_classobj, _classname, ctor, _classname, nitMethod, ctor->GetFlags());
			_hasCtor = true;
		} else TVPAddLog(ttstr(TJS_W("Multiple constructors.(")) + _classname + TJS_W(")"));
		return *this;
	}

	/// �v���p�e�B��o�^����
	template <typename GetterT, typename SetterT>
	ThisClassT &Property(NameT name, GetterT g, SetterT s) {
		typedef ncbNativeClassProperty<GetterT, SetterT> PropT;
		PropT *prop = new PropT(g, s);
		TJSNativeClassRegisterNCM(_classobj, name, prop, _classname, nitProperty, prop->GetFlags());
		return *this;
	}
	template <typename GetterT>
	ThisClassT &Property(NameT name, GetterT g, int) {
		typedef typename dummyProperty<GetterT>::SetterT SetterT;
		return Property(name, g, static_cast<SetterT>(0));
	}
	template <typename SetterT>
	ThisClassT &Property(NameT name, int, SetterT s) {
		typedef typename dummyProperty<SetterT>::GetterT GetterT;
		return Property(name, static_cast<SetterT>(0), s);
	}

	/// RawCallback
	template <typename MethodT>
	ThisClassT &MethodRawCallback(NameT name, MethodT m, ncbNativeClassMethodBase::FlagsT flag) {
		typedef typename rawCallbackSelect<NativeClassT, MethodT>::Type CallbackObjectT;
		TJSNativeClassRegisterNCM(_classobj, name, CallbackObjectT::CreateObject(m, flag), _classname, nitMethod, flag);
		return *this;
	}

private:
	bool          _hasCtor;		//< �R���X�g���N�^��o�^������
	NameT         _classname;	//< �N���X��
	IdentT        _classid;		//< �N���XID
	ClassObjectT *_classobj;	//< �N���X�I�u�W�F�N�g

	template <typename A, typename B, typename C, typename D>
	struct typeSelector { typedef D Type; };

	template <typename A, typename C, typename D>
	struct typeSelector<A, A, C, D> { typedef C Type; };

	template <typename P>
	struct dummyProperty {
		typedef typename MethodCaller::tMethodTraits<P>::ClassType ClassT;
		typedef typename typeSelector<ClassT, void, void (*)(), void (ClassT::*)() >::Type MethodT;
		typedef MethodT GetterT;
		typedef MethodT SetterT;
	};

	template <class CLS, typename M> struct rawCallbackSelect                                     { typedef ncbRawCallbackMethod<CLS>  Type; };
	template <class CLS>             struct rawCallbackSelect<CLS, tTJSNativeClassMethodCallback> { typedef ncbRawCallbackMethod<void> Type; };
};


////////////////////////////////////////
/// TJS�N���X�������W�X�g�N���X�i�X���b�h�A���Z�[�t�Ȃ̂ŕK�v�Ȃ�K���ɏC���̂��Ɓj
// ��{�I�� static const �ȃC���X�^���X�����g�p����Ȃ��̂ł���ŏ\���ȋC�͂���
struct ncbAutoRegister {
	typedef ncbAutoRegister ThisClassT;
	typedef void (*CallBackT)();

	enum LineT {
		PreRegist = 0,
		ClassRegist,
		PostRegist,
		LINE_COUNT };
#define NCB_INNER_AUTOREGISTER_LINES_INSTANCE { 0, 0, 0 }

	ncbAutoRegister(LineT line) : _next(_top[line]) { _top[line] = this; }

	static void AllRegist(  LineT line) { NCB_LOG_2(TJS_W("AllRegist:"),   line); for (ThisClassT const* p = _top[line]; p; p = p->_next) p->Regist();   }
	static void AllUnregist(LineT line) { NCB_LOG_2(TJS_W("AllUnregist:"), line); for (ThisClassT const* p = _top[line]; p; p = p->_next) p->Unregist(); }

	static void AllRegist()   { for (int line = 0; line < LINE_COUNT; line++) AllRegist(  static_cast<LineT>(line)); }
	static void AllUnregist() { for (int line = 0; line < LINE_COUNT; line++) AllUnregist(static_cast<LineT>(line)); }
protected:
	virtual void Regist()   const = 0;
	virtual void Unregist() const = 0;
private:
	ncbAutoRegister();
	/****/ ThisClassT const* _next;
	static ThisClassT const* _top[LINE_COUNT];

protected:
	/// NCB_METHOD_DETAIL�Ń��\�b�h�ڍׂ̃^�C�v�����肷�邽�߂Ɏg�p
	struct TagClassMethod {};
	struct TagConstMethod {};
	struct TagStaticMethod {};
	template <class TAG, typename METHOD> struct MethodType;

	/// ���\�b�h�v�f�𕪉����ďꍇ�킯���č\�z����
#define NCB_INNER_CREATE_CLASS_METHODTYPE(tag, type) \
	template <typename METHOD> struct MethodType<tag, METHOD> { \
		typedef MethodCaller CallerT; \
		typedef CallerT::tMethodTraits<METHOD> TraitsT; \
		typedef typename CallerT::tMethodResolver< \
			typename TraitsT::ResultType, type, typename TraitsT::ArgsType >::Type Type; }
	NCB_INNER_CREATE_CLASS_METHODTYPE(TagClassMethod,  typename TraitsT::ClassType);
	NCB_INNER_CREATE_CLASS_METHODTYPE(TagConstMethod,  typename TraitsT::ClassType const);
	NCB_INNER_CREATE_CLASS_METHODTYPE(TagStaticMethod, void);
};

template <class T>
struct ncbClassAutoRegister : public ncbAutoRegister {
	typedef T ClassT;
	typedef ncbCreateClass<ClassT> RegistT;
	typedef ncbClassAutoRegister<ClassT> ThisClassT;
	typedef const tjs_char *NameT;

	ncbClassAutoRegister(NameT n) : _name(n), ncbAutoRegister(ClassRegist)  {}
protected:
	void Regist()   const { RegistT r(_name); _Regist(r); }
	void Unregist() const { ncbCreateClass<T>::Release(); }
private:
	NameT _name;
	static void _Regist(RegistT &);
};

#define NCB_REGISTER_CLASS(cls) \
	NCB_TYPECONV_BOXING(cls); \
	template    struct ncbClassInfo<cls>; \
	template <>        ncbClassInfo<cls>::InfoT ncbClassInfo<cls>::_info; \
	static ncbClassAutoRegister<cls> ncbClassAutoRegister_ ## cls(TJS_W(# cls)); \
	template <> void   ncbClassAutoRegister<cls>::_Regist(ncbClassAutoRegister<cls>::RegistT &_r_)

#define NCB_CONSTRUCTOR(cargs) _r_.Constructor(RegistT::ConstructorArgs<void (ClassT::*) cargs >())

#define NCB_METHOD_DIFFER(name, method) _r_.Method(TJS_W(# name), &ClassT::method)
#define NCB_METHOD(method) NCB_METHOD_DIFFER(method, method)

// tag = { Class, Const, Statc }
#define NCB_METHOD_DETAIL(name, tag, result, method, args) \
	_r_.Method(TJS_W(# name), static_cast<MethodType<Tag ## tag ## Method, result (ClassT::*) args >::Type >(&ClassT::method))

#define NCB_METHOD_RAW_CALLBACK(name, cb, flag) _r_.MethodRawCallback(TJS_W(# name), cb, flag)

#define NCB_PROPERTY(name,get,set) _r_.Property(TJS_W(# name), &ClassT::get, &ClassT::set)
#define NCB_PROPERTY_RO(name,get)  _r_.Property(TJS_W(# name), &ClassT::get, (int)0)
#define NCB_PROPERTY_WO(name,set)  _r_.Property(TJS_W(# name), (int)0, &ClassT::set)



/*
	�}�N���g�p���@��

NCB_REGISTER_CLASS(�N���X) {
	NCB_CLASS_DEF("�N���X��", (�R���X�g���N�^����1,����2, ...));
	NCB_METHOD(���\�b�h��1);
	NCB_METHOD(���\�b�h��2);
		:
	NCB_METHOD(���\�b�h��n);

	// ���\�b�h���͂��̂܂�"���\�b�h��"�Ƃ��ăN���X�ɓo�^����܂�
	// �Ⴄ���O�œo�^����������
	NCB_METHOD(���O, ���\�b�h��n);

	// �����ň����̈Ⴄ���\�b�h������ꍇ�͈ȉ��̃}�N�����g���Ă�������
	NCB_METHOD_DETAIL_M(�Ԃ�l�^, ���\�b�h��, (����1,����2, ...)); // ���ʂ̃��\�b�h�̏ꍇ
	NCB_DETAIL_C(�Ԃ�l�^, ���\�b�h��, (����1,����2, ...)); // const ���\�b�h�̏ꍇ
	NCB_DETAIL_S(�Ԃ�l�^, ���\�b�h��, (����1,����2, ...)); // static���\�b�h�̏ꍇ

	// �v���p�e�B��o�^����ꍇ�͈ȉ��̃}�N�����g�p���܂�
	// setter��getter���\�b�h��static�Ɣ�static�����݂���ƃG���[�ɂȂ�܂�
	// getter�̕Ԃ�l��setter�̈���������т��̌��̃`�F�b�N�͂���Ȃ��̂Œ��ӂ��Ă�������
	NCB_PROPERTY   (�v���p�e�B��, getter���\�b�h��, setter���\�b�h��);
	NCB_PROPERTY_RO(�v���p�e�B��, getter���\�b�h��);
	NCB_PROPERTY_WO(�v���p�e�B��, setter���\�b�h��);
	// (RO = Read Only, WO = Write Only)

}

 */


////////////////////////////////////////
/// TJS�t�@���N�V�����������W�X�g�N���X

template <typename METHOD>
struct ncbNativeFunction : public ncbNativeClassMethodBase {
	typedef METHOD MethodT;
	/// constructor
	ncbNativeFunction(MethodT m)
		: ncbNativeClassMethodBase(TJS_W("Function")), // TJS�I�u�W�F�N�g�I�ɂ� Function
		  _method(m)
	{
		if (!_method) TVPThrowExceptionMessage(TJS_W("No function pointer."));
	}
	/// FuncCall����
	tjs_error  TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint, 
		tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		// �������g���Ă΂ꂽ�̂ł͂Ȃ��ꍇ�̓G���[
		if (membername) return TJS_E_MEMBERNOTFOUND;
		// ���\�b�h�Ăяo��
		return Invoke(_method, objthis, result, numparams, param);
	}
private:
	MethodT const _method;
};


struct ncbFunctionAutoRegister : public ncbAutoRegister {
	typedef tjs_char const*           NameT;

	ncbFunctionAutoRegister() : ncbAutoRegister(ClassRegist)  {}
protected:
	void Regist()   const {}
	void Unregist() const {}

	template <typename MethodT>
	static void RegistFunction(  NameT name, MethodT const &m) {
		typedef ncbNativeFunction<MethodT> MethodObjectT;

		iTJSDispatch2 * global = TVPGetScriptDispatch();
		if (!global) return;

		MethodObjectT *mobj = new MethodObjectT(m);
		if (mobj) {
			tTJSVariant val(mobj);
			mobj->Release();
			global->PropSet(TJS_MEMBERENSURE, name, 0, &val, global);
		}
		global->Release();
	}
	static void UnregistFunction(NameT name) {
		iTJSDispatch2 * global = TVPGetScriptDispatch();
		if (!global) return;
		global->DeleteMember(0, name, 0, global);
		global->Release();
	}
};

template <typename DUMMY>
struct ncbFunctionAutoRegisterTempl;

#define NCB_REGISTER_FUNCTION(name, function) \
	struct ncbFunctionTag_ ## name {}; \
	template <> struct ncbFunctionAutoRegisterTempl<ncbFunctionTag_ ## name> : public ncbFunctionAutoRegister \
	{	typedef        ncbFunctionAutoRegisterTempl<ncbFunctionTag_ ## name> ThisClassT; \
		void Regist()   const { RegistFunction(TJS_W(# name), function); } \
		void Unregist() const { UnregistFunction(TJS_W(# name)); } }; \
	static ncbFunctionAutoRegisterTempl<ncbFunctionTag_ ## name> ncbFunctionAutoRegister_ ## name




////////////////////////////////////////
/// ���W�X�g�O��̃R�[���o�b�N�o�^
struct ncbCallbackAutoRegister : public ncbAutoRegister {
	typedef void (*CallbackT)();

	ncbCallbackAutoRegister(LineT line, CallbackT init, CallbackT term)
		: _init(init), _term(term), ncbAutoRegister(line)  {}
protected:
	void Regist()   const { if (_init) _init(); }
	void Unregist() const { if (_term) _term(); }
private:
	CallbackT _init, _term;
};

#define NCB_REGISTER_CALLBACK(pos, init, term, tag) static ncbCallbackAutoRegister ncbCallbackAutoRegister_ ## pos ## _ ## tag (ncbAutoRegister::pos, init, term)
#define NCB_PRE_REGIST_CALLBACK(cb)    NCB_REGISTER_CALLBACK(PreRegist,  &cb, 0, cb ## _0)
#define NCB_POST_REGIST_CALLBACK(cb)   NCB_REGISTER_CALLBACK(PostRegist, &cb, 0, cb ## _0)
#define NCB_PRE_UNREGIST_CALLBACK(cb)  NCB_REGISTER_CALLBACK(PreRegist,  0, &cb, 0_ ## cb)
#define NCB_POST_UNREGIST_CALLBACK(cb) NCB_REGISTER_CALLBACK(PostRegist, 0, &cb, 0_ ## cb)


#endif
