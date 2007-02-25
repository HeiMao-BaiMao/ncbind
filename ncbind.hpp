#ifndef _ncbind_hpp_
#define _ncbind_hpp_

#include <windows.h>
#include "tp_stub.h"
#include "ncb_invoke.hpp"

////////////////////////////////////////
// ���O�o�͗p�}�N��

#define NCB_WARN(n)     TVPAddLog(ttstr(n))
#define NCB_WARN_2(a,b) TVPAddLog(ttstr(a) + ttstr(b))
#define NCB_WARN_W(str) NCB_WARN(TJS_W(str))

#if (defined(DEBUG) || defined(_DEBUG))
#define NCB_LOG(n)     NCB_WARN(n)
#define NCB_LOG_2(a,b) NCB_WARN_2(a,b)
#define NCB_LOG_W(str) NCB_WARN_W(str)
#else
#define NCB_LOG_VOID   ((void)0)
#define NCB_LOG(n)     NCB_LOG_VOID
#define NCB_LOG_2(a,b) NCB_LOG_VOID
#define NCB_LOG_W(str) NCB_LOG_VOID
#endif


////////////////////////////////////////
// ���ʌ^��`
struct ncbTypedefs {
	typedef tjs_char const*           NameT;
	typedef tjs_uint32                FlagsT;
	typedef tjs_int32                 IdentT;

	typedef MethodCaller              CallerT;
	typedef tTJSNativeInstanceType    InstanceTypeT;
	typedef tTJSNativeClassForPlugin  ClassObjectT;

	/// �^�̎󂯓n���Ŏg�p
	template <typename T> struct Tag { typedef T Type; };

	/// �ꍇ�킯�Ŏg�p
	template <int  N> struct NumTag  { enum { n = N }; };
	template <bool B> struct BoolTag { enum { b = B }; };

	// tTJSVariant::Type() ���b�p (�����ł����̂�����������)
	static inline tTJSVariantType GetVariantType(tTJSVariant const &var) { return (const_cast<tTJSVariant*>(&var))->Type(); }
//	static inline tTJSVariantType GetVariantType(tTJSVariant &var)       { return var.Type(); }

	// �R�[���o�b�N�^
	typedef tTJSNativeClassMethodCallback CallbackT;

	// �C���X�^���X�ɕϊ����ēn���R�[���o�b�N
	template <class T> 
	struct CallbackWithInstance {
		typedef tjs_error (TJS_INTF_METHOD    *Type)(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, T *nativeInstance);
	};

	template <class A, class B> struct TypeEqual      { enum { NotEqual, Result = false }; };
	template <class A>          struct TypeEqual<A,A> { enum {    Equal, Result = true  }; };
	template <bool F, typename ERR> struct TypeAssert            { typedef void Result; };
	template <typename ERR>         struct TypeAssert<true, ERR> { typedef typename ERR::CompileError Result; };
};

////////////////////////////////////////
/// NativeClass ���O/ID/�N���X�I�u�W�F�N�g�ێ��p
template <class T>
struct ncbClassInfo {
	typedef T NativeClassT;

	typedef ncbTypedefs::NameT        NameT;
	typedef ncbTypedefs::IdentT       IdentT;
	typedef ncbTypedefs::ClassObjectT ClassObjectT;

	/// �v���p�e�B�擾
	static inline NameT         GetName()        { return _info.name; }
	static inline IdentT        GetID()          { return _info.id; }
	static inline ClassObjectT *GetClassObject() { return _info.obj; }

	/// �C�j�V�����C�U
	static inline bool Set(NameT name, IdentT id, ClassObjectT *obj) {
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
		ClassObjectT *obj;
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
	static AdaptorT *GetAdaptor(iTJSDispatch2 *obj, bool err = false) {
		iTJSNativeInstance* adp = 0;
		if (!obj) {
			if (err) TVPThrowExceptionMessage(TJS_W("No instance."));
			return 0;
		}
		if (TJS_FAILED(obj->NativeInstanceSupport(TJS_NIS_GETINSTANCE, ClassInfoT::GetID(), &adp))) {
			if (err) TVPThrowExceptionMessage(TJS_W("Invalid instance type."));
			return 0;
		}
		return static_cast<AdaptorT*>(adp);
	}

	/// iTJSDispatch2 ���� NativeClass�C���X�^���X���擾
	static NativeClassT *GetNativeInstance(iTJSDispatch2 *obj, bool err = false) {
		AdaptorT *adp = GetAdaptor(obj, err);
		return adp ? adp->_instance : 0;
	}

	/// NativeClass�C���X�^���X��ݒ�
	static bool SetNativeInstance(iTJSDispatch2 *obj, NativeClassT *instance, bool err = false) {
		AdaptorT *adp = GetAdaptor(obj, err);
		if (!adp) return false;
		adp->_instance = instance;
		return true;
	}

	/// �A�_�v�^�𐶐�����NativeClass�C���X�^���X��ݒ�
	static bool SetAdaptorWithNativeInstance(iTJSDispatch2 *obj, NativeClassT *instance, bool err = false) {
		AdaptorT *adp = GetAdaptor(obj, false);
		if (adp) {
			if (adp->_instance) adp->_deleteInstance();
		} else if (!(adp = new AdaptorT())) {
			if (err) TVPThrowExceptionMessage(TJS_W("Create adaptor failed."));
			return false;
		}
		adp->_instance = instance;
		iTJSNativeInstance *ni = static_cast<iTJSNativeInstance*>(adp);
		if (TJS_FAILED(obj->NativeInstanceSupport(TJS_NIS_REGISTER, ClassInfoT::GetID(), &ni))) {
			if (err) TVPThrowExceptionMessage(TJS_W("Adaptor registration failed."));
			return false;
		}
		return true;
	}

	/// �N���X�I�u�W�F�N�g����Adaptor�C���X�^���X�𐶐�����instance����
	static iTJSDispatch2* CreateAdaptor(NativeClassT *inst, bool err = false) {
		typename ClassInfoT::ClassObjectT *clsobj = ClassInfoT::GetClassObject();
		if (!clsobj) {
			if (err) TVPThrowExceptionMessage(TJS_W("No class object."));
			return 0;
		}

		iTJSDispatch2 *global = TVPGetScriptDispatch(), *obj = 0;
		tTJSVariant dummy, *param = &dummy;
		// ������1�ł���void�ł���Ύ��C���X�^���X��new���Ȃ�����ɂȂ�
		tjs_error r = clsobj->CreateNew(0, 0, 0, &obj, 1, &param, global);
		if (global) global->Release();

		if (TJS_FAILED(r) || !obj) {
			if (err) TVPThrowExceptionMessage(TJS_W("Can't create instance"));
			return 0;
		}
		AdaptorT *adp = GetAdaptor(obj, err);
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

	/// FROM ���� TO �֕ϊ��ł��邩
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
	template <typename T> struct ToTarget             { static T& Get(T *t) { return *t; } };
	template <typename T> struct ToTarget<T&>         { static T& Get(T *t) { return *t; } };
	template <typename T> struct ToTarget<T*>         { static T* Get(T *t) { return  t; } };

	/// const�O��
	template <typename T> struct NonConst             { typedef T  Type; };
	template <typename T> struct NonConst<const T>    { typedef T  Type; };
	template <typename T> struct NonConst<const T&>   { typedef T& Type; };
	template <typename T> struct NonConst<const T*>   { typedef T* Type; };

	/// reference �O��
	template <typename T> struct NonReference         { typedef T Type; };
	template <typename T> struct NonReference<T&>     { typedef T Type; };

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

	// �^���ꉻ�����݂��邩�̃}�b�v�p
	template <typename T, bool SRCF>
	struct SpecialMap {
		enum { Exists = false, Modifier = false, IsSource = SRCF };
		typedef T Type;
	};

	/// ���얢��i�R���p�C���G���[�p�j
	struct NCB_COMPILE_ERROR_NoImplement;

	// �R���o�[�^����I��
	struct SelectConvertorTypeBase {
	protected:
		/// �R�����Z�q
		template <bool EXP, class THEN, class ELSE> struct ifelse                   { typedef ELSE Type; };
		template <          class THEN, class ELSE> struct ifelse<true, THEN, ELSE> { typedef THEN Type; };

		/// ���ꉻ�����݂��邩���ׂ�e���v���[�g
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
			typedef typename ifelse<Conversion<SRC, DST>::Exists, DirectCopy, NCB_COMPILE_ERROR_NoImplement>::Type Type;
		};
	};

	/// �R���o�[�^�̃^�C�v�𒲂ׂ�
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
// ncbTypeConvertor::Conversion �̓��ꉻ
       NCB_INNER_CONVERSION_SPECIALIZATION
#undef NCB_INNER_CONVERSION_SPECIALIZATION

//--------------------------------------
// TypeConvertor�֘A�̊e��}�N��

/// SpecialMap �ɓo�^����}�N��
#define NCB_TYPECONV_MAPSET(mapsel, type, conv, mod) \
	template <> struct ncbTypeConvertor::SpecialMap<type, mapsel> { \
		enum { Exists = true, Modifier = mod, IsSource = mapsel }; \
		typedef conv Type; \
	}
#define NCB_TYPECONV_SRCMAP_SET(type, conv, mod) NCB_TYPECONV_MAPSET(true,  type, conv, mod)
#define NCB_TYPECONV_DSTMAP_SET(type, conv, mod) NCB_TYPECONV_MAPSET(false, type, conv, mod)

/// DirectCopy����Ƃ��ă}�b�v�ɓo�^
#define NCB_TYPECONV_DIRECT(type) \
	NCB_TYPECONV_SRCMAP_SET(type, ncbTypeConvertor::DirectCopy, false); \
	NCB_TYPECONV_DSTMAP_SET(type, ncbTypeConvertor::DirectCopy, false)

/// Cast����Ƃ��ă}�b�v�ɓo�^
#define NCB_TYPECONV_CAST(type, cast) \
	NCB_TYPECONV_SRCMAP_SET(type, ncbTypeConvertor::CastCopy<cast>, false); \
	NCB_TYPECONV_DSTMAP_SET(type, ncbTypeConvertor::CastCopy<cast>, false)

/// ���l�L���X�g�œo�^
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
	/// Constructor (���\�b�h���Ă΂��O)
	ncbVariatToNChar() : _nstr(0) {}
	/// Destructor (���\�b�h���Ă΂ꂽ��)
	~ncbVariatToNChar() {
		if (_nstr) {
//			NCB_LOG_W("~ncbVariatToNChar > delete[]");
			delete[] _nstr;
		}
	}
	/// ���������󂯓n�����߂̃t�@���N�^
	template <typename DST>
	inline void operator()(DST &dst, tTJSVariant const &src) {
		// �Ȃ� tTJSVariant::Type() �� const ���\�b�h�łȂ��̂��c
		if (ncbTypedefs::GetVariantType(src) == tvtString) {
			tTJSString s(src.AsStringNoAddRef());
			tjs_int len = s.GetNarrowStrLen();

//			NCB_LOG_W("ncbVariatToNChar::operator() > new tjs_nchar[]");
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

/// std::string�Ȃǂ� c_str() �Ŏ󂯓n��
template <class StringT>
struct ncbStringConvertor {
	typedef ncbTypedefs      DefsT;
	typedef ncbTypeConvertor ConvT;
	typedef DefsT::NumTag<sizeof(typename StringT::value_type)> SizeTagT;

	template <class STR>
	inline void operator()(tTJSVariant &dst, STR const &src) const {
		dst = (ConvT::ToPointer<STR const&>::Get(src))->c_str();
	}
	template <class STR>
	inline void operator()(STR &dst, tTJSVariant const &src) {
		if (ncbTypedefs::GetVariantType(src) == tvtString) {
			tTJSString str(src.AsStringNoAddRef());
			set(str, SizeTagT());
		}
		dst = ConvT::ToTarget<STR>::Get(&_temp);
	}
	inline void set(tTJSString const &str, DefsT::NumTag<sizeof(tjs_nchar)>) { // for Narrow char
		tjs_int len = str.GetNarrowStrLen();
		tjs_nchar tmp[len+1];
		str.ToNarrowStr(tmp, len+1);
		_temp.assign(tmp, len);
	}
	inline void set(tTJSString const &str, DefsT::NumTag<sizeof(tjs_char)>) { // for Wide char
		_temp = str.c_str();
	}
private:
	StringT _temp;
};
#define NCB_TYPECONV_STL_STRING(type) \
	NCB_TYPECONV_SRCMAP_SET(type, ncbStringConvertor<type>, true); \
	NCB_TYPECONV_DSTMAP_SET(type, ncbStringConvertor<type>, true)


/// �l�C�e�B�u�C���X�^���X�A�_�v�^�� Boxing/Unboxing ����
struct ncbNativeObjectBoxing {
	typedef tTJSVariant VarT;
	typedef ncbTypeConvertor ConvT;
	/// Boxing
	struct Boxing {
		template <typename SRC>
		inline void operator ()(VarT &dst, SRC &src) const {
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
	struct Unboxing {
		template <typename DST>
		inline void operator ()(DST &dst, VarT const &src) const {
			typedef DST                                     TargetT;
			typedef typename ConvT::Stripper<TargetT>::Type ClassT;
			typedef ncbInstanceAdaptor<ClassT>              AdaptorT;

			iTJSDispatch2 *obj = src.AsObjectNoAddRef();			//< �Q�ƃJ�E���^�����Ȃ���Dispatch�I�u�W�F�N�g�擾
			ClassT *p = AdaptorT::GetNativeInstance(obj, true);		//< ���C���X�^���X�̃|�C���^���擾
			dst = ConvT::ToTarget<TargetT>::Get(p);					//< �K�v�Ƃ����^�ɕϊ����ĕԂ�
		}
	};
};

/// �{�b�N�X������^�Ƃ��ēo�^����}�N��
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

// iTJSDispatch2*���������E�Ԃ�l�ɂ���ꍇ
struct ncbDispatchConvertor {
	inline void operator ()(tTJSVariant &dst, iTJSDispatch2* &src) const {
		dst = tTJSVariant(src, src);
		src->Release();
	}
	inline void operator ()(iTJSDispatch2* &dst, tTJSVariant const &src) const {
		dst = src.AsObjectNoAddRef();
	}
	inline void operator ()(iTJSDispatch2 const* &dst, tTJSVariant const &src) const {
		dst = src.AsObjectNoAddRef();
	}
};
NCB_SET_TOVARIANT_CONVERTOR(iTJSDispatch2*,       ncbDispatchConvertor);
NCB_SET_TOVALUE_CONVERTOR(  iTJSDispatch2*,       ncbDispatchConvertor);
NCB_SET_TOVALUE_CONVERTOR(  iTJSDispatch2 const*, ncbDispatchConvertor);


/*
	�^�ϊ��𒼂ŏ�����������

	struct CustomType; // �ϊ�����Ώۂ̌^
	struct CustomConvertor { // �R���o�[�^
		void operator ()(tTJSVariant &dst, CustomType const &src);
		void operator ()(CustomType const &src, tTJSVariant &dst);
	};
	NCB_SET_CONVERTOR(CustomType, CustomConvertor);
	�Ƃ������悤�Ȋ����œK����
 */

////////////////////////////////////////
/// ���\�b�h�I�u�W�F�N�g�i�ƁC���̃^�C�v�ƃt���O�j���󂯓n�����߂̃C���^�[�t�F�[�X
struct ncbIMethodObject {
	typedef iTJSDispatch2*             DispatchT;
	typedef ncbTypedefs::FlagsT        FlagsT;
	typedef ncbTypedefs::InstanceTypeT TypesT;

	virtual DispatchT GetDispatch() const = 0;
	virtual FlagsT    GetFlags()    const = 0;
	virtual TypesT    GetType()     const = 0;
	virtual void      Release()     const = 0;
};

////////////////////////////////////////
/// ���\�b�h�Ăяo���p�x�[�X�N���X

struct ncbNativeClassMethodBase : public tTJSDispatch {
	typedef tTJSDispatch             BaseT;
	typedef ncbNativeClassMethodBase ThisClassT;
	typedef ncbTypedefs              DefsT;
	typedef DefsT::NameT             NameT;
	typedef DefsT::FlagsT            FlagsT;
	typedef DefsT::InstanceTypeT     TypesT;
	typedef ncbIMethodObject const*  iMethodT;
	typedef tTJSNativeInstanceType   MethodTypeT;

	/// constructor
	ncbNativeClassMethodBase(MethodTypeT t) : _type(t), _name(0) {
		_imethod = this;
		switch (t) { // �^�C�v����ݒ�
//		case nitClass:    _name = TJS_W("Class");    break; // �N���X�ɂȂ邱�Ƃ͂��肦�Ȃ�
		case nitMethod:   _name = TJS_W("Function"); break;
		case nitProperty: _name = TJS_W("Property"); break;
		default: break;
		}
	}
	~ncbNativeClassMethodBase() {}

	/// IsInstanceOf ����
	tjs_error TJS_INTF_METHOD IsInstanceOf(
		tjs_uint32 flag, const tjs_char *membername, tjs_uint32 *hint, 
		const tjs_char *classname, iTJSDispatch2 *objthis)
	{
		// �������g(membername==0)�Ŕ�r�N���X����_name�Ȃ�TRUE�C����ȊO�͊ۓ���
		return (!membername && _name && !TJS_stricmp(classname, _name)) ? TJS_S_TRUE
			: BaseT::IsInstanceOf(flag, membername, hint, classname, objthis);
	}

private:
	MethodTypeT const _type;
	NameT _name;

	// private�ŉB�����Ă݂�
	typedef DefsT::CallerT CallerT;

	//--------------------------------------
	/// TJSNativeClassRegisterNCM �ɓn���^�C�v
	virtual TypesT GetType() const { return _type; }

	/// TJSNativeClassRegisterNCM �ɓn���t���O
	virtual FlagsT GetFlags() const { return 0; }

	/// IMethod ����
	struct iMethod : public ncbIMethodObject {
		typedef ncbNativeClassMethodBase MethodObjectT;
		void operator = (MethodObjectT *mo) { _this = mo; }
		DispatchT GetDispatch() const { return static_cast<DispatchT>(_this); }
		FlagsT    GetFlags()    const { return _this->GetFlags(); }
		TypesT    GetType()     const { return _this->GetType(); }
		void      Release()     const {}
	private:
		MethodObjectT *_this;
	} _imethod;

protected:
	/// IMethod �擾
	iMethodT GetIMethod() const { return &_imethod; }



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
	/// ���\�b�h�^����t���O�����肷�邽�߂̃w���p�e���v���[�g
private:
	template <typename T> struct flagsSelector       { enum { Flags = 0 }; };
#define NCB_INNER_FLAGSELECTOR_SPECIALIZATION \
	template <>           struct ncbNativeClassMethodBase::flagsSelector<void> { enum { Flags = TJS_STATICMEMBER }; };

protected:
	template <typename T> 
	struct methodFlags { enum { Flags = flagsSelector<typename traits<T>::ClassT>::Flags }; };


	//--------------------------------------
private:
	/// ����/�Ԃ�l�̈����n���p�t�@���N�^ (ncb_invoke.hpp/MethodCaller�ɓn�����߂ɕK�v)
	template <typename RES, class ARGS, int ARGC>
	struct paramsFunctor {
		typedef tTJSVariant      VariantT;
		typedef ncbTypeConvertor ConvT;
		typedef typename     ConvT::SelectConvertorType<RES, VariantT>::Type ResultConvT;

		template <typename T> struct TypeWrap           { typedef T        Type; static inline T        Restore(Type t) { return  t; } };
		template <typename T> struct TypeWrap<T&>       { typedef T*       Type; static inline T&       Restore(Type t) { return *t; } };
		template <typename T> struct TypeWrap<T const&> { typedef T const* Type; static inline T const& Restore(Type t) { return *t; } };

		template <typename T> struct ArgsExtor { typedef typename ConvT::SelectConvertorType<VariantT, typename TypeWrap<T>::Type>::Type Type; };
		typedef CallerT::tArgsExtract<ArgsExtor, ARGS, ARGC> ArgsConvT;
		/* ArgsConvT �͈ȉ��̂悤�ɓW�J�����F
			struct ArgsConvT {
				ncbToValueConvertor<ARGS::Arg1Type> t1; // ��Ԗڂ̈����� ncbToValueConvertor
				ncbToValueConvertor<ARGS::Arg2Type> t2; // ��Ԗڂ�
				  :
				ncbToValueConvertor<ARGS::Arg[ARGC]Type> tARGC; // ARGC�Ԗڂ�
			}; */

		/// constructor
		paramsFunctor(VariantT *r, tjs_int n, VariantT const *const *p) : _numparams(n), _result(r),_param(p) {}

		/// ������ NativeClassMethod �ֈ��n��
		template <int N, typename T>
		inline T operator ()(CallerT::tNumTag<N> const& /*index*/, CallerT::tTypeTag<T> const& /*type*/) {
			typedef typename TypeWrap<T>::Type ParamT;
			ParamT ret;
			// N�Ԗڂ� ncbToValueConvertor �����o���ĕϊ�
			(CallerT::tArgsSelect<ArgsConvT, N>::Get(_aconv))(ret, (_numparams >= N) ? *(_param[N - 1]) : VariantT());
			return TypeWrap<T>::Restore(ret);
		}

		/// NativeClassMethod �̕Ԃ�l��result�֊i�[
		template <typename ResultT>
		inline void operator = (ResultT r) {
			// ncbToVariantConvertor �ŕԂ�l�ɕϊ�
			if (_result) _rconv(*_result, r);
		}
	private:
		// �^�ϊ��p���[�N
		ArgsConvT   _aconv;
		ResultConvT _rconv;

		// �����E�Ԃ�l�p�����[�^
		tjs_int                _numparams;
		VariantT             * _result;
		VariantT const *const* _param;
	};

	// �擪�ɃC���X�^���X�|�C���^��n��PROXY METHOD�̃t�@���N�^
	template <class CLASS, typename RES, class ARGS, int ARGC>
	struct paramsFunctorWithInstance : public paramsFunctor<RES, ARGS, ARGC> {
		typedef                               paramsFunctor<RES, ARGS, ARGC> BaseT;
		typedef CLASS            ClassT;
		typedef tTJSVariant      VariantT;
		paramsFunctorWithInstance(ClassT *inst, VariantT *r, tjs_int n, VariantT const *const *p) : BaseT(r, n+1, p-1), _inst(inst) {}
		template <int N, typename T> inline T operator ()(CallerT::tNumTag<N> const &idx, CallerT::tTypeTag<T> const &tag) { return BaseT::operator ()(idx, tag); }
		template <       typename T> inline T operator ()(CallerT::tNumTag<1> const &,    CallerT::tTypeTag<T> const &   ) { return _inst; }
		template <typename ResultT>  inline void operator = (ResultT r) { return BaseT::operator =(r); }
	private:
		ClassT *_inst;
	};


	//--------------------------------------
private:
	/// �l�C�e�B�u�C���X�^���X���擾���邽�߂̃e���v���[�g�i���ꉻ�ŏ㏑���ł���悤�ɂ��邽��
	template <class T>
	struct nativeInstanceGetterBase {
		typedef T  ClassT;
		typedef ncbInstanceAdaptor<ClassT> AdaptorT;
		typedef iTJSDispatch2* DispatchT;
		typedef tjs_error      ErrorT;

		nativeInstanceGetterBase() : _error(TJS_S_OK) {}

		inline ClassT *GetNativeInstance(DispatchT objthis) {
			ClassT *r = AdaptorT::GetNativeInstance(objthis); // ���C���X�^���X�̃|�C���^
			if (!r) SetError(TJS_E_NATIVECLASSCRASH);
			return r;
		}
		inline bool SetNativeInstance(DispatchT objthis, ClassT *obj) {
			SetError(TJS_S_OK);
			return AdaptorT::SetAdaptorWithNativeInstance(objthis, obj);
		}
		inline ErrorT  GetError() const   { return _error; }
		inline void    SetError(ErrorT e) { _error = e; }

		/// �f�t�H���g����
		inline ClassT* Get(DispatchT objthis) { return GetNativeInstance(objthis); }
	private:
		ErrorT _error;
	};

	/// �w�肪�Ȃ��ꍇ�̓f�t�H���g����iBase��Get���Ă΂��j
	template <class T>
	struct nativeInstanceGetter : public nativeInstanceGetterBase<T> {};

	//--------------------------------------
protected:
	/// ���\�b�h/�R���X�g���N�^�Ăяo����try/catch�����ݍ��ނ��߂̃w���p�e���v���[�g
	template <bool  IsAny>  struct invokeHookAll   { template <typename T> static inline typename T::ResultT Do(T &t) { return t(); } };
	template <class ClassT> struct invokeHookClass { template <typename T> static inline typename T::ResultT Do(T &t) { return invokeHookAll<T::Hook>::Do(t); } };

	//--------------------------------------
protected:
	// �t�b�N�̈������ɓn���t�@���N�^�̃x�[�X�i�p�����[�^�ێ��j
	struct doInvokeBase {
		typedef tjs_error ResultT;
		enum { Hook = true };

		typedef tTJSVariant  * RetT;
		typedef tjs_int        NumT;
		typedef tTJSVariant ** ArgsT;
		typedef iTJSDispatch2            *ObjT;
		doInvokeBase(RetT r, NumT n, ArgsT p, ObjT o)
			: _result(r), _numparams(n), _param(p), _objthis(o) { if (r) r->Clear(); }
	protected:
		RetT  const _result;
		NumT  const _numparams;
		ArgsT const _param;
		ObjT  const _objthis;

		/// ���\�b�h�^���� paramsFunctor �^�����肷��e���v���[�g
		template <typename METHOD>
		struct methodUtil {
			typedef traits<METHOD>           TraitsT;
			typedef typename TraitsT::ClassT ClassT;
			enum {      ArgsCount = TraitsT::ArgsCount };
			typedef paramsFunctor<
				typename TraitsT::ResultT,
			/**/typename TraitsT::ArgsT, ArgsCount > FunctorT;
		};
	};
	// �N���X���\�b�h�Ăяo���t�@���N�^
	template <class REFCLASS, typename METHOD, typename METHODCLASS>
	struct doInvoke : public doInvokeBase {
		typedef METHOD MethodT;
		typedef methodUtil<MethodT> UtilT;
		typedef REFCLASS RefClassT;

		typedef typename UtilT::ClassT ClassT;
		typedef nativeInstanceGetter<ClassT> InstanceGetterT;

		doInvoke(MethodT const &m, RetT r, NumT n, ArgsT p, ObjT o) : doInvokeBase(r, n, p, o), _m(m) {}
		inline ResultT operator()() const {
			// �������̌������Ȃ��ꍇ�̓G���[
			if (_numparams < UtilT::ArgsCount) return TJS_E_BADPARAMCOUNT;

			// �C���X�^���X�|�C���^���擾
			InstanceGetterT g;
			ClassT *obj = g.Get(_objthis);
			ResultT   r = g.GetError();
			if (TJS_FAILED(r)) return r;

			// �C���X�^���X��n���ăN���X���\�b�h�����s
			typename UtilT::FunctorT fnct(_result, _numparams, _param);
			return CallerT::Invoke(fnct, _m, obj) ? TJS_S_OK : TJS_E_FAIL;
		}
	private:
		MethodT const &_m;
	};
	// static���\�b�h�Ăяo���t�@���N�^
	template <class REFCLASS, typename METHOD>
	struct doInvoke<REFCLASS, METHOD, void> : public doInvokeBase {
		typedef METHOD MethodT;
		typedef methodUtil<MethodT> UtilT;
		typedef REFCLASS RefClassT;

		doInvoke(MethodT const &m, RetT r, NumT n, ArgsT p, ObjT o) : doInvokeBase(r, n, p, o), _m(m) {}
		inline ResultT operator()() const {
			// �������̌������Ȃ��ꍇ�̓G���[
			if (_numparams < UtilT::ArgsCount) return TJS_E_BADPARAMCOUNT;

			typename UtilT::FunctorT fnct(_result, _numparams, _param);
			return CallerT::Invoke(fnct, _m) ? TJS_S_OK : TJS_E_FAIL;
		}
	private:
		MethodT const &_m;
	};

	// tTJSNativeClassMethodCallback �ȏꍇ�̃t�@���N�^
	template <class REFCLASS>
	struct doInvoke<REFCLASS, tTJSNativeClassMethodCallback, void> : public doInvokeBase {
		typedef tTJSNativeClassMethodCallback MethodT;
		typedef REFCLASS RefClassT;

		doInvoke(MethodT const &m, RetT r, NumT n, ArgsT p, ObjT o) : doInvokeBase(r, n, p, o), _m(m) {}
		inline ResultT operator()() const {
			return _m(_result, _numparams, _param, _objthis);
		}
	private:
		MethodT const &_m;
	};
#if 0 //�Ƃ肠�����ۗ�
	// �C���X�^���X�ɕϊ����R�[���o�b�N �ȏꍇ�̃t�@���N�^
	template <class CLASS>
	struct doInvoke<CLASS, typename DefsT::CallbackWithInstance<REFCLASS>::Type> : public doInvokeBase {
		typedef /*          */typename DefsT::CallbackWithInstance<REFCLASS>::Type MethodT;
		typedef tjs_error ResultT;
		typedef CLASS     ClassT;
		typedef ClassT RefClassT;
		typedef nativeInstanceGetter<ClassT> InstanceGetterT;

		doInvoke(MethodT const &m, RetT r, NumT n, ArgsT p, ObjT o) : doInvokeBase(r, n, p, o), _m(m) {}
		inline ResultT operator()() const {
			// �C���X�^���X�|�C���^���擾
			InstanceGetterT g;
			ClassT *obj = g.Get(_objthis);
			ResultT   r = g.GetError();
			if (TJS_FAILED(r)) return r;

			return _m(_result, _numparams, _param, _objthis);
		}
	private:
		MethodT const &_m;
	};
#endif

	//////
	// ���\�b�hProxy�Ăяo���t�@���N�^
	template <class REFCLASS, typename METHOD>
	struct doProxy : public doInvokeBase {
		typedef METHOD MethodT;
		typedef methodUtil<MethodT> UtilT;
		typedef REFCLASS RefClassT;
		typedef REFCLASS ClassT;
		typedef paramsFunctorWithInstance<ClassT,
		/**/typename UtilT::TraitsT::ResultT,
		/**/typename UtilT::TraitsT::ArgsT, UtilT::ArgsCount > FunctorT;

		struct MustBeStaticMethod;
		typedef typename DefsT::TypeAssert<!DefsT::TypeEqual<void, typename UtilT::ClassT>::Result, MustBeStaticMethod>::Result MethodCheck;

		typedef nativeInstanceGetter<ClassT> InstanceGetterT;

		doProxy(MethodT const &m, RetT r, NumT n, ArgsT p, ObjT o) : doInvokeBase(r, n, p, o), _m(m) {}
		inline ResultT operator()() const {
			// �������̌������Ȃ��ꍇ�̓G���[
			if (_numparams < UtilT::ArgsCount -1) return TJS_E_BADPARAMCOUNT;

			// �C���X�^���X�|�C���^���擾
			InstanceGetterT g;
			ClassT *obj = g.Get(_objthis);
			ResultT   r = g.GetError();
			if (TJS_FAILED(r)) return r;

			// �C���X�^���X��n���ăN���X���\�b�h�����s
			FunctorT fnct(obj, _result, _numparams, _param);
			return CallerT::Invoke(fnct, _m) ? TJS_S_OK : TJS_E_FAIL;
		}
	private:
		MethodT const &_m;
	};

	//--------------------------------------
public:
	/// ���\�b�h�Ăяo�������
	template <typename InvokeT>
	static inline tjs_error Invoke(InvokeT const &invoker) {
		// ���\�b�h�Ăяo��
		return invokeHookClass<typename InvokeT::RefClassT>::Do(invoker);
	}
	template <class RefClassT, typename MethodT>
	struct invokeSelector {
		typedef doInvoke<RefClassT, MethodT, typename traits<MethodT>::ClassT> Type;
		static inline FlagsT GetFlags() { return methodFlags<MethodT>::Flags; }
	};
	template <class RefClassT, typename MethodT>
	struct proxySelector {
		typedef doProxy< RefClassT, MethodT> Type;
		static inline FlagsT GetFlags() { return 0; }
	};


	//////
	// �R���X�g���N�^�Ăяo���t�@���N�^
	template <typename METHOD>
	struct doConstruct : public doInvokeBase {
		typedef METHOD MethodT;
		typedef methodUtil<MethodT> UtilT;

		typedef typename UtilT::ClassT ClassT;
		typedef ClassT* ResultT;
		typedef ncbInstanceAdaptor<ClassT> AdaptorT;
		enum { ArgsCount = traits<MethodT>::ArgsCount };

		doConstruct(RetT r, NumT n, ArgsT p, ObjT o) : doInvokeBase(r, n, p, o) {}
		inline ResultT operator ()() const {
			typename UtilT::FunctorT fnct(_result, _numparams, _param);
			return CallerT::Factory(fnct, CallerT::tTypeTag<MethodT>());
		}
		inline tjs_error Construct() const {
			// �������̌������Ȃ��ꍇ�̓G���[
			if (_numparams < UtilT::ArgsCount) return TJS_E_BADPARAMCOUNT;

			// �������g�̃t�@���N�^�Ăяo��
			ClassT* instance = 0;
			if (!(instance = invokeHookClass<ClassT>::Do(*this))) {
				TVPThrowExceptionMessage(TJS_W("NativeClassInstance creation faild."));
				return TJS_E_FAIL;
			}

			// �G���[�������ǂ��Ȃ�̂��悭�킩��񂯂ǓK���Ɂi��
			tjs_error r = TJS_S_OK;
			try {
				if (!AdaptorT::SetNativeInstance(_objthis, instance)) {
					r = TJS_E_NATIVECLASSCRASH;
					delete instance;
				}
			} catch (...) {
				if (instance) delete instance;
				throw;
			}
			return r;
		}
	};
};
/// ncbNativeClassMethodBase::flagsSelector���ꉻ(���ꉻ�̓N���X��`���ɂ͋L�q�ł��Ȃ��炵��)
       NCB_INNER_FLAGSELECTOR_SPECIALIZATION
#undef NCB_INNER_FLAGSELECTOR_SPECIALIZATION



////////////////////////////////////////
/// ���\�b�h�Ăяo���N���X�e���v���[�g
// �{���� TJSCreateNativeClassMethod�i�y�ыg���g������tTJSNativeClassMethod�j���g�p����Ƃ����
// ���O�Ŏ�������(TJSCreateNativeClassMethod�ł�static�Ȋ֐������ĂׂȂ��̂Ń��\�b�h�ւ̃|�C���^�̕ێ�������Ȃ���)
template <class RefClassT, typename METHOD,
/*      */template <class, typename> class SELECTOR = ncbNativeClassMethodBase::invokeSelector> // doInvoke / doProxy �̑I��
struct ncbNativeClassMethod : public ncbNativeClassMethodBase { 
	typedef ncbNativeClassMethod ThisClassT;
	typedef METHOD MethodT;
	typedef SELECTOR<RefClassT, MethodT> SelectorT;

	/// constructor
	ncbNativeClassMethod(MethodT m) : ncbNativeClassMethodBase(nitMethod), _method(m) {
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
		typedef typename SelectorT::Type InvokeT;
		InvokeT const invoker(_method, result, numparams, param, objthis);
		return Invoke(invoker);
	}
	/// factory
	static iMethodT Create(MethodT m) { return (new ThisClassT(m))->GetIMethod(); }

protected:
	MethodT const _method;

private:
	/// TJSNativeClassRegisterNCM�t���O
	FlagsT GetFlags() const { return SelectorT::GetFlags(); }
};

template <class RefClassT, typename METHOD>
struct ncbNativeClassMethodProxyType {
	typedef ncbNativeClassMethod<RefClassT, METHOD, ncbNativeClassMethodBase::proxySelector> Type;
};

////////////////////////////////////////
/// �R���X�g���N�^�Ăяo���N���X�e���v���[�g
template <typename METHOD>
struct ncbNativeClassConstructor : public ncbNativeClassMethodBase {
	typedef ncbNativeClassConstructor ThisClassT;
	typedef METHOD MethodT;

	/// constructor
	ncbNativeClassConstructor() : ncbNativeClassMethodBase(nitMethod) {}

	/// FuncCall����
	tjs_error  TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint, 
		tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		// �������g���Ă΂ꂽ�̂ł͂Ȃ��ꍇ�͊ۓ���
		if (membername) return BaseT::FuncCall(flag, membername, hint, result, numparams, param, objthis);

		// ���������ЂƂł���void�̏ꍇ�̓C���X�^���X��ݒ肵�Ȃ�
		if ((numparams == 1) && (ncbTypedefs::GetVariantType(*param[0]) == tvtVoid)) {
			NCB_LOG_W("Constructor(void)");
			return TJS_S_OK;
		}
		// �l�C�e�B�u�C���X�^���X����
		doConstruct<MethodT> const factory(result, numparams, param, objthis);
		return factory.Construct();
	}

	/// iMethod factory
	static iMethodT Create() { return (new ThisClassT())->GetIMethod(); }
};


////////////////////////////////////////

template <class RefClassT, typename GETTER, typename SETTER,
/*      */template <class, typename> class SELECTOR = ncbNativeClassMethodBase::invokeSelector> // doInvoke / doProxy �̑I��
struct ncbNativeClassProperty : public ncbNativeClassMethodBase {
	typedef ncbNativeClassProperty ThisClassT;
	typedef GETTER GetterT;
	typedef SETTER SetterT;
	typedef SELECTOR<RefClassT, GetterT> GetterSelT;
	typedef SELECTOR<RefClassT, SetterT> SetterSelT;

	/// constructor
	ncbNativeClassProperty(GetterT get, SetterT set) : ncbNativeClassMethodBase(nitProperty), _getter(get), _setter(set) {}

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
		typedef typename GetterSelT::Type InvokeT;
		InvokeT const get(_getter, result, 0, 0, objthis);
		return Invoke(get);
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
		typedef typename SetterSelT::Type InvokeT;
		InvokeT const set(_setter, 0, 1, const_cast<tTJSVariant**>(&param), objthis);
		return Invoke(set);
	}

	/// factory
	static iMethodT Create(GetterT g, SetterT s) { return (new ThisClassT(g, s))->GetIMethod(); }

private:
	/// Getter �� Setter �������N���X�̃��\�b�h���ǂ����𔻒�i�Ⴄ�ꍇ�̓R���p�C���G���[�j
	enum { COMPILE_ERROR_PropertyMethodMustBeSameClass = DefsT::TypeEqual<
		typename traits<GetterT>::ClassT,
		typename traits<SetterT>::ClassT >::Equal
	};

	/// TJSNativeClassRegisterNCM�t���O
	FlagsT GetFlags() const { return GetterSelT::GetFlags(); }

protected:
	/// �v���p�e�B���\�b�h�ւ̃|�C���^
	GetterT const _getter;
	SetterT const _setter;
};

template <class RefClassT, typename GETTER, typename SETTER>
struct ncbNativeClassPropertyProxyType {
	typedef ncbNativeClassProperty<RefClassT, GETTER, SETTER, ncbNativeClassMethodBase::proxySelector> Type;
};

////////////////////////////////////////
/// ���R�[���o�b�N�p
template <typename T> struct ncbRawCallbackMethod;

// �l�C�e�B�u�C���X�^���X�̃|�C���^�݂̂��炩���ߎ擾����R�[���o�b�N
template <class T>
struct ncbRawCallbackMethod<
/*        */tjs_error (TJS_INTF_METHOD *         )(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, T *nativeInstance) > : public ncbNativeClassMethodBase {
	typedef tjs_error (TJS_INTF_METHOD *CallbackT)(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, T *nativeInstance);
	typedef T                                NativeClassT;
	typedef ncbRawCallbackMethod             ThisClassT;
	typedef ncbInstanceAdaptor<NativeClassT> AdaptorT;


	/// constructor
	ncbRawCallbackMethod(CallbackT m, FlagsT f)
		: ncbNativeClassMethodBase(nitMethod), // TJS�I�u�W�F�N�g�I�ɂ� Function
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

	/// factory
	static iMethodT Create(CallbackT cb, FlagsT f) { return (new ThisClassT(cb, f))->GetIMethod(); }

protected:
	CallbackT const _callback;
	FlagsT    const _flag;

private:
	FlagsT    GetFlags()    const { return _flag; }
};

/// �]���� TJSCreateNativeClassMethod�p ncbIMethodObject���b�p
template <>
struct ncbRawCallbackMethod<tTJSNativeClassMethodCallback> : public ncbIMethodObject {
	typedef ncbRawCallbackMethod ThisClassT;
	typedef tTJSNativeClassMethodCallback CallbackT;
	typedef ncbNativeClassMethodBase::iMethodT iMethodT;

	ncbRawCallbackMethod(CallbackT m, FlagsT f) : _dispatch(TJSCreateNativeClassMethod(m)), _flags(f) {}

	DispatchT GetDispatch() const { return _dispatch; }
	FlagsT    GetFlags()    const { return _flags; }
	TypesT    GetType()     const { return nitMethod; }
	void      Release()     const { delete this; }

	/// factory
	static iMethodT Create(CallbackT cb, FlagsT f) { return new ThisClassT(cb, f); }
private:
	DispatchT const _dispatch;
	FlagsT    const _flags;
};


////////////////////////////////////////
/// �v���p�e�B�p RawCallback

// ncbRawCallbackMethod �� AccessDenied ��Ԃ��_�~�[�^�̃Z���N�^
template <typename CallbackT>
struct ncbRawCallbackPropertySelector {
	typedef ncbRawCallbackMethod<CallbackT> Type;
};
template <>
struct ncbRawCallbackPropertySelector<int> {
	typedef struct AccessDenied {
		AccessDenied(int, ncbNativeClassMethodBase::FlagsT) {}
		tjs_error  TJS_INTF_METHOD FuncCall(
			tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint, 
			tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
		{
			return TJS_E_ACCESSDENYED;
		}
	} Type;
};

// ncbRawCallbackMethod �����˂Ď���
template <typename GETTER, typename SETTER>
struct ncbRawCallbackProperty : public ncbNativeClassMethodBase {
	typedef ncbRawCallbackProperty ThisClassT;
	typedef GETTER GetCallbackT;
	typedef SETTER SetCallbackT;
	typedef typename ncbRawCallbackPropertySelector<GetCallbackT>::Type GetterT;
	typedef typename ncbRawCallbackPropertySelector<SetCallbackT>::Type SetterT;

	/// constructor
	ncbRawCallbackProperty(GetCallbackT get, SetCallbackT set, FlagsT f)
		: ncbNativeClassMethodBase(nitProperty), // TJS�I�u�W�F�N�g�I�ɂ� Property
		  _getter(get, f), _setter(set, f), _flag(f) {} 

	/// PropGet ����
	tjs_error TJS_INTF_METHOD PropGet(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint, 
		tTJSVariant *result, iTJSDispatch2 *objthis)
	{
		// �������g���Ă΂ꂽ�̂ł͂Ȃ��ꍇ�͊ۓ���
		if (membername) return BaseT::PropGet(flag, membername, hint, result, objthis);
		// ���\�b�h�Ăяo��
		return _getter.FuncCall(flag, membername, hint, result, 0, 0, objthis);
	}

	/// PropSet ����
	tjs_error TJS_INTF_METHOD PropSet(
		tjs_uint32 flag, const tjs_char *membername, tjs_uint32 *hint, 
		const tTJSVariant *param, iTJSDispatch2 *objthis)
	{
		// �������g���Ă΂ꂽ�̂ł͂Ȃ��ꍇ�͊ۓ���
		if (membername) return BaseT::PropSet(flag, membername, hint, param, objthis);
		// ���\�b�h�Ăяo��
		return _setter.FuncCall(flag, membername, hint, 0, 1, &param, objthis);
	}

	/// TJSNativeClassRegisterNCM�t���O
	FlagsT GetFlags() const { return _flag; }

	/// factory
	static iMethodT Create(GetCallbackT get, SetCallbackT set, FlagsT f) { return (new ThisClassT(get, set, f))->GetIMethod(); }
protected:
	GetterT _getter;
	SetterT _setter;
	FlagsT _flag;
};


////////////////////////////////////////
/// NativeClass �N���X�I�u�W�F�N�g�o�^�e���v���[�g

template <class DELG>
struct ncbRegistClass {
	typedef ncbTypedefs DefsT;
	typedef DELG DelegateT;
	typedef typename DelegateT::NameT  NameT;
	typedef typename DelegateT::FlagsT FlagsT;
	typedef typename DelegateT::ItemT  ItemT;

	ncbRegistClass(DelegateT &d, bool r) : _delegator(d), _isRegist(r), _emptyItem(DelegateT::GetEmptyItem()) { Begin(); }
	~ncbRegistClass() { End(); }
private:
	DelegateT &_delegator;
	bool  const _isRegist;
	ItemT const _emptyItem;

	typedef DefsT::BoolTag<false> NonProxyT;

public:
	void Begin()					{ if (_isRegist) _delegator.RegistBegin();    else _delegator.UnregistBegin(); }
	void End()						{ if (_isRegist) _delegator.RegistEnd();      else _delegator.UnregistEnd(); }
	void DoItem(NameT n, ItemT t)	{ if (_isRegist) _delegator.RegistItem(n, t); else _delegator.UnregistItem(n); }
	NameT GetName() const			{ return _delegator.GetName(); }

	/// ���\�b�h��o�^����
	template <typename MethodT, bool IsProxy>
	void Method(NameT name, MethodT m, DefsT::BoolTag<IsProxy>) {
		typedef typename DelegateT::template Method<MethodT, IsProxy>::Type TargetT;
		DoItem(name, _isRegist ? TargetT::Create(m) : _emptyItem);
	}
	// defaults (non-proxy)
	template <typename MethodT> void Method(NameT name, MethodT m) { Method(name, m, NonProxyT()); }

	/// �R���X�g���N�^��o�^����
	template <typename MethodT>
	void Constructor(ncbTypedefs::Tag<MethodT>) {
		typedef typename DelegateT::template Constructor<MethodT>::Type TargetT;
		DoItem(GetName(), _isRegist ? TargetT::Create() : _emptyItem);
	}

	/// �v���p�e�B��o�^����
	template <typename GetterT, typename SetterT, bool IsProxy>
	void Property(NameT name, GetterT g, SetterT s, DefsT::BoolTag<IsProxy>) {
		typedef typename DelegateT::template Property<GetterT, SetterT, IsProxy>::Type TargetT;
		DoItem(name, _isRegist ? TargetT::Create(g, s) : _emptyItem);
	}
	// ReadOnly / WriteOnly Properties.
	template <typename GetterT, bool IsProxy> void Property(NameT name, GetterT g, int, DefsT::BoolTag<IsProxy> const &tag) { Property(name, g, static_cast<GetterT>(0), tag); }
	template <typename SetterT, bool IsProxy> void Property(NameT name, int, SetterT s, DefsT::BoolTag<IsProxy> const &tag) { Property(name, static_cast<SetterT>(0), s, tag); }
	// defaults (non-proxy)
	template <typename GetterT, typename SetterT> void Property(NameT name, GetterT g, SetterT s) { Property(name, g, s, NonProxyT()); }
	template <typename GetterT>                   void Property(NameT name, GetterT g, int s)     { Property(name, g, s, NonProxyT()); }
	template <typename SetterT>                   void Property(NameT name, int g, SetterT s)     { Property(name, g, s, NonProxyT()); }

	/// RawCallback Method
	template <typename MethodT>
	void MethodRawCallback(NameT name, MethodT m, FlagsT flags) {
		typedef typename DelegateT::template RawCallback<MethodT>::Type TargetT;
		DoItem(name, _isRegist ? TargetT::Create(m, flags) : _emptyItem);
	}

	/// RawCallback Property
	template <typename GetterT, typename SetterT>
	void PropertyRawCallback(NameT name, GetterT g, SetterT s, FlagsT flags) {
		typedef typename DelegateT::template RcbProperty<GetterT, SetterT>::Type TargetT;
		DoItem(name, _isRegist ? TargetT::Create(g, s, flags) : _emptyItem);
	}
};

////////////////////////////////////////

struct ncbRegistNativeClassBase {
	typedef ncbNativeClassMethodBase::NameT      NameT;
	typedef ncbNativeClassMethodBase::iMethodT   ItemT;
	typedef ncbNativeClassMethodBase::FlagsT     FlagsT;
	ncbRegistNativeClassBase(NameT name) : _className(name) {}

	void   RegistBegin()			{}
	void   RegistItem(NameT, ItemT)	{}
	void   RegistEnd()				{}
	void UnregistBegin()			{}
	void UnregistItem(NameT)		{}
	void UnregistEnd()				{}

	NameT  GetName() const {return _className; }
	static inline ItemT GetEmptyItem() { return 0; }
protected:
	NameT const _className;
};

template <class CLASS>
struct ncbRegistNativeClass : public ncbRegistNativeClassBase {
	typedef                          ncbRegistNativeClassBase BaseT;
	typedef CLASS                                NativeClassT;
	typedef ncbInstanceAdaptor<NativeClassT>     AdaptorT;

	typedef ncbClassInfo<NativeClassT>           ClassInfoT;
	typedef typename ClassInfoT::IdentT          IdentT;
	typedef typename ClassInfoT::ClassObjectT    ClassObjectT;

	/// �o�^�ɑΉ�����^�𓱏o����e���v���[�g�Q
	template <typename T>                      struct Constructor  { typedef ncbNativeClassConstructor<T>               Type; };
	template <typename T, bool PX>             struct Method       { typedef ncbNativeClassMethod<NativeClassT, T>      Type; };
	template <typename G, typename S, bool PX> struct Property     { typedef ncbNativeClassProperty<NativeClassT, G, S> Type; };
	template <typename T>                      struct RawCallback  { typedef ncbRawCallbackMethod<T>                    Type; };
	template <typename G, typename S>          struct RcbProperty  { typedef ncbRawCallbackProperty<G, S>               Type; };
	// proxy type
	template <typename T>             struct Method<T, true>      { typedef typename ncbNativeClassMethodProxyType<  NativeClassT, T   >::Type Type; };
	template <typename G, typename S> struct Property<G, S, true> { typedef typename ncbNativeClassPropertyProxyType<NativeClassT, G, S>::Type Type; };

	ncbRegistNativeClass(NameT n) : BaseT(n), _classobj(0), _hasCtor(false) {}

	void RegistBegin() {
		NCB_LOG_2(TJS_W("BeginRegistClass: "), _className);

		// �N���X�I�u�W�F�N�g�𐶐�
		_classobj = TJSCreateNativeClassForPlugin(_className, AdaptorT::CreateEmptyAdaptor);

		// �N���XID�𐶐�
		IdentT id  = TJSRegisterNativeClass(_className);

		// ncbClassInfo�ɓo�^
		if (!ClassInfoT::Set(_className, id, _classobj))
			TVPThrowExceptionMessage(TJS_W("Already registerd class."));

		// �N���X�I�u�W�F�N�g��ID��ݒ�
		TJSNativeClassSetClassID(_classobj, id);

		// ���finalize���\�b�h��ǉ�
		TJSNativeClassRegisterNCM(_classobj, TJS_W("finalize"),
								  TJSCreateNativeClassMethod(EmptyCallback),
								  _className, nitMethod);
	}

	void RegistItem(NameT name, ItemT item) {
		NCB_LOG_2(TJS_W("  RegistItem: "), name);
		if (name == _className) {
			if (_hasCtor) TVPAddLog(tTJSString(TJS_W("Multiple constructors.(")) + _className + TJS_W(")"));
			_hasCtor = true;
		}
		if (item) {
			TJSNativeClassRegisterNCM(_classobj, name, item->GetDispatch(), _className, item->GetType(), item->GetFlags());
			item->Release();
		}
	}

	void RegistEnd() {
		// �R���X�g���N�^���Ȃ��ꍇ�̓G���[��Ԃ��R���X�g���N�^��ǉ�����
		if (!_hasCtor) TJSNativeClassRegisterNCM(_classobj, _className,
												 TJSCreateNativeClassMethod(NotImplCallback),
												 _className, nitMethod);

		// TJS �̃O���[�o���I�u�W�F�N�g���擾����
		iTJSDispatch2 *global = TVPGetScriptDispatch();
		if (!global) {
			NCB_WARN_W("No Global Dispatch, Regist failed.");
			return;
		}

		// 2 _classobj �� tTJSVariant �^�ɕϊ�
		tTJSVariant val(static_cast<iTJSDispatch2*>(_classobj));

		// 3 ���ł� val �� _classobj ��ێ����Ă���̂ŁA_classobj �� Release ����
		//_classobj->Release();
		// �c�Ƃ������Ƃ��낾���CBoxing�����ŃC���X�^���X�����֌W�ŃN���X�I�u�W�F�N�g���K�v�Ȃ̂ŊJ�����Ȃ�
		// (�v���O�C���J�����Ƀ����[�X�����ncbCreateClass::Release)

		// 4 global �� PropSet ���\�b�h��p���A�I�u�W�F�N�g��o�^����
		global->PropSet(
			TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
			_className, // �����o��
			0, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
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

		NCB_LOG_2(TJS_W("EndRegistClass: "), _className);
	}

	/// �v���O�C���J�����ɃN���X�I�u�W�F�N�g�������[�X����
	void UnregistBegin() {
		iTJSDispatch2 *global = TVPGetScriptDispatch();
		typename ClassInfoT::ClassObjectT *obj = ClassInfoT::GetClassObject();

		if (global) {
			global->DeleteMember(0, _className, 0, global);
			global->Release();
		}
		if (obj) obj->Release();

		ClassInfoT::Clear();
	}

private:
	ClassObjectT *_classobj;	//< �N���X�I�u�W�F�N�g
	bool          _hasCtor;		//< �R���X�g���N�^��o�^������

	/// ��̃��\�b�h(finalize Callback�p)
	static tjs_error TJS_INTF_METHOD EmptyCallback(  tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) { return TJS_S_OK; }

	/// �G���[��Ԃ����\�b�h(empty Constructor�p)
	static tjs_error TJS_INTF_METHOD NotImplCallback(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) { return TJS_E_NOTIMPL; }
};

////////////////////////////////////////
/// �����N���X�I�u�W�F�N�g��ύX����

template <class CLASS>
struct ncbAttachTJS2Class : public ncbRegistNativeClassBase {
	typedef                        ncbRegistNativeClassBase BaseT;
	typedef CLASS                                NativeClassT;
	typedef ncbClassInfo<NativeClassT>           ClassInfoT;
	typedef typename ClassInfoT::IdentT          IdentT;
	typedef typename ClassInfoT::ClassObjectT    ClassObjectT;

	/// �o�^�ɑΉ�����^�𓱏o����e���v���[�g�Q
	struct NCB_COMPILE_ERROR_CantSetConstructor; // �R���X�g���N�^�͑��݂ł��Ȃ��̂ŃR���p�C���G���[���o��
	template <typename T>                      struct Constructor  { typedef NCB_COMPILE_ERROR_CantSetConstructor       Type; };

	template <typename T, bool PX>             struct Method       { typedef ncbNativeClassMethod<NativeClassT, T>      Type; };
	template <typename G, typename S, bool PX> struct Property     { typedef ncbNativeClassProperty<NativeClassT, G, S> Type; };
	template <typename T>                      struct RawCallback  { typedef ncbRawCallbackMethod<T>                    Type; };
	template <typename G, typename S>          struct RcbProperty  { typedef ncbRawCallbackProperty<G, S>               Type; };
	// proxy type
	template <typename T>             struct Method<T, true>      { typedef typename ncbNativeClassMethodProxyType<  NativeClassT, T   >::Type Type; };
	template <typename G, typename S> struct Property<G, S, true> { typedef typename ncbNativeClassPropertyProxyType<NativeClassT, G, S>::Type Type; };

	ncbAttachTJS2Class(NameT nativeClass, NameT tjs2Class) : BaseT(nativeClass), _tjs2ClassName(tjs2Class) {}

	void RegistBegin() {
		NCB_LOG_2(TJS_W("BeginAttachTJS2Class: "), _className);

		// TJS �̃O���[�o���I�u�W�F�N�g���擾����
		_global = TVPGetScriptDispatch();
		_tjs2ClassObj = GetGlobalObject(_tjs2ClassName, _global);

		// �N���XID�𐶐�
		IdentT id  = TJSRegisterNativeClass(_className);
		NCB_LOG_2(TJS_W("  ID: "), (tjs_int)id);

		// ncbClassInfo�ɓo�^
		if (!ClassInfoT::Set(_className, id, 0)) {
			TVPThrowExceptionMessage(TJS_W("Already registerd class."));
		}
	}

	void RegistItem(NameT name, ItemT item) {
		NCB_LOG_2(TJS_W("  RegistItem: "), name);
		if (!item) return;
		iTJSDispatch2 *dsp = item->GetDispatch();
		tTJSVariant val(dsp);
		dsp->Release();
		FlagsT flg = item->GetFlags();
		_tjs2ClassObj->PropSet(TJS_MEMBERENSURE | flg, name, 0, &val, ((flg & TJS_STATICMEMBER) ? _global : _tjs2ClassObj));
		item->Release();
	}

	void RegistEnd() {
		NCB_LOG_2(TJS_W("EndAttachClass: "), _className);
		if (_global) _global->Release();
		_global = 0;
		// _tjs2ClassObj �� NoAddRef �Ȃ̂� Release �s�v
	}

	void UnregistBegin() {
		// �N���X�I�u�W�F�N�g���擾
		iTJSDispatch2 *global = TVPGetScriptDispatch();
		_tjs2ClassObj = GetGlobalObject(_tjs2ClassName, global);
		if (global) global->Release();
	}
	void UnregistItem(NameT name) {
		_tjs2ClassObj->DeleteMember(0, name, 0, _tjs2ClassObj);
	}
	void UnregistEnd() {
		ClassInfoT::Clear();
		// _tjs2ClassObj �� NoAddRef �Ȃ̂� Release �s�v
	}
protected:
	NameT const    _tjs2ClassName;
	iTJSDispatch2* _tjs2ClassObj;	//< �N���X�I�u�W�F�N�g
	iTJSDispatch2* _global;

	// �N���X�I�u�W�F�N�g���擾
	static iTJSDispatch2* GetGlobalObject(NameT name, iTJSDispatch2 *global) {
		tTJSVariant val;
		if (global) global->PropGet(0, name, 0, &val, global);
		else {
			NCB_WARN_W("No Global Dispatch.");
			TVPExecuteExpression(name, &val);
		}
		return val.AsObjectNoAddRef();
	}
};




////////////////////////////////////////
/// TJS�N���X�������W�X�g�N���X�i�X���b�h�A���Z�[�t�Ȃ̂ŕK�v�Ȃ�K���ɏC���̂��Ɓj
// ��{�I�� static const �ȃC���X�^���X�����g�p����Ȃ��̂ł���ŏ\���ȋC�͂���
struct ncbAutoRegister {
	typedef ncbAutoRegister ThisClassT;
	typedef void (*CallBackT)();
	typedef ncbTypedefs::NameT NameT;
	typedef ncbTypedefs::BoolTag<true>  TrueTagT;
	typedef ncbTypedefs::BoolTag<false> FalseTagT;

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

////////////////////////////////////////
template <class T>
struct  ncbNativeClassAutoRegister : public ncbAutoRegister {
	/**/ncbNativeClassAutoRegister(NameT n) : ncbAutoRegister(ClassRegist), _name(n) {}
private:
	typedef T ClassT;
	typedef T Class;
	typedef ncbRegistNativeClass<T> DelegateT;
	typedef ncbRegistClass<DelegateT> RegistT;

	NameT const _name;
	static void _Regist(RegistT &);
protected:
	void Regist()   const { DelegateT d(_name); { RegistT r(d, true);  _Regist(r); } }
	void Unregist() const { DelegateT d(_name); { RegistT r(d, false); _Regist(r); } }
};

#define NCB_REGISTER_CLASS_COMMON(cls, tmpl, init) \
	template    struct ncbClassInfo<cls>; \
	template <>        ncbClassInfo<cls>::InfoT ncbClassInfo<cls>::_info; \
	static tmpl<cls> tmpl ## _ ## cls init; \
	void   tmpl<cls>::_Regist (RegistT &_r_)

#define NCB_REGISTER_CLASS_DELAY(name, cls) \
	NCB_REGISTER_CLASS_COMMON(cls, ncbNativeClassAutoRegister, (TJS_W(# name)))

#define NCB_REGISTER_CLASS(cls) NCB_REGISTER_CLASS_DIFFER(cls, cls)
#define NCB_REGISTER_CLASS_DIFFER(name, cls) \
	NCB_TYPECONV_BOXING(cls); \
	NCB_REGISTER_CLASS_COMMON(cls, ncbNativeClassAutoRegister, (TJS_W(# name)))

////////////////////////////////////////
template <class T>
struct  ncbAttachTJS2ClassAutoRegister : public ncbAutoRegister {
	/**/ncbAttachTJS2ClassAutoRegister(NameT ncn, NameT tjscn) : ncbAutoRegister(ClassRegist), _nativeClassName(ncn), _tjs2ClassName(tjscn) {}
protected:
	typedef T ClassT;
	typedef T Class;
	typedef ncbAttachTJS2Class<T>     DelegateT;
	typedef ncbRegistClass<DelegateT> RegistT;

	void Regist()   const { DelegateT d(_nativeClassName, _tjs2ClassName); { RegistT r(d, true);  _Regist(r); } }
	void Unregist() const { DelegateT d(_nativeClassName, _tjs2ClassName); { RegistT r(d, false); _Regist(r); } }
private:
	NameT const _nativeClassName;
	NameT const _tjs2ClassName;
	static void _Regist(RegistT &);
};

#define NCB_GET_INSTANCE_HOOK(cls) \
	template <> struct ncbNativeClassMethodBase::nativeInstanceGetter<cls> : public ncbNativeClassMethodBase::nativeInstanceGetterBase<cls>

#define NCB_GET_INSTANCE_HOOK_CLASS nativeInstanceGetter

#define NCB_INSTANCE_GETTER(objthis) \
	inline ClassT* Get(DispatchT objthis)

#define NCB_ATTACHED_INSTANCE_DELAY_CREATE(cls) \
	NCB_GET_INSTANCE_HOOK(cls) { NCB_INSTANCE_GETTER(objthis) { \
		ClassT* obj = GetNativeInstance(objthis); \
		if (!obj) { obj = new ClassT(); SetNativeInstance(objthis, obj); } \
		return obj; } }

#define NCB_ATTACH_CLASS_WITH_HOOK(cls, attach) \
	template <> struct ncbNativeClassMethodBase::nativeInstanceGetter<cls>; \
	NCB_REGISTER_CLASS_COMMON(cls, ncbAttachTJS2ClassAutoRegister, (TJS_W(# cls), TJS_W(# attach)))

#define NCB_ATTACH_CLASS(cls, attach) \
	NCB_ATTACHED_INSTANCE_DELAY_CREATE(cls); \
	NCB_ATTACH_CLASS_WITH_HOOK(cls, attach)

////////////////////////////////////////
#define NCB_CONSTRUCTOR(cargs) _r_.Constructor(ncbTypedefs::Tag<void (ClassT::*) cargs >())

#define NCB_METHOD_DIFFER(name, method) _r_.Method(TJS_W(# name), &ClassT::method)
#define NCB_METHOD(method) NCB_METHOD_DIFFER(method, method)

#define NCB_METHOD_CAST(tag, result, method, args) \
	static_cast<MethodType<Tag ## tag ## Method, result (ClassT::*) args >::Type >(&method)

// tag = { Class, Const, Statc }
#define NCB_METHOD_DETAIL(name, tag, result, method, args) \
	_r_.Method(TJS_W(# name), NCB_METHOD_CAST(tag, result, method, args))

#define NCB_METHOD_RAW_CALLBACK(name, cb, flag) _r_.MethodRawCallback(TJS_W(# name), cb, flag)

#define NCB_PROPERTY(name,get,set) _r_.Property(TJS_W(# name), &ClassT::get, &ClassT::set)
#define NCB_PROPERTY_RO(name,get)  _r_.Property(TJS_W(# name), &ClassT::get, (int)0)
#define NCB_PROPERTY_WO(name,set)  _r_.Property(TJS_W(# name), (int)0, &ClassT::set)

#define NCB_PROPERTY_DETAIL(name, r_tag, r_result, r_method, r_args, w_tag, w_result, w_method, w_args) \
	_r_.Property(TJS_W(# name), NCB_METHOD_CAST(r_tag, r_result, r_method, r_args), NCB_METHOD_CAST(w_tag, w_result, w_method, w_args))

#define NCB_PROPERTY_DETAIL_RO(name, tag, result, method, args) _r_.Property(TJS_W(# name), NCB_METHOD_CAST(tag, result, method, args), (int)0)
#define NCB_PROPERTY_DETAIL_WO(name, tag, result, method, args) _r_.Property(TJS_W(# name), (int)0, NCB_METHOD_CAST(tag, result, method, args))

#define NCB_PROPRETY_RAW_CALLBACK(name, get, set, flag) _r_.PropertyRawCallback(TJS_W(# name), get,    set, flag)
#define NCB_PROPRETY_RAW_CALLBACK_RO(name, get, flag)   _r_.PropertyRawCallback(TJS_W(# name), get, (int)0, flag)
#define NCB_PROPRETY_RAW_CALLBACK_WO(name, set, flag)   _r_.PropertyRawCallback(TJS_W(# name), (int)0, set, flag)

// ���������Ȃ񂾂��킩��Ȃ��Ȃ��Ă��܂����i��
#define NCB_METHOD_PROXY(name, method) /*                      */_r_.Method(TJS_W(# name), &method, TrueTagT())
#define NCB_METHOD_PROXY_DETAIL(name, tag, result, method, args) _r_.Method(TJS_W(# name), NCB_METHOD_CAST(tag, result, method, args), TrueTagT())
#define NCB_PROPERTY_PROXY(name,get,set) /*                    */_r_.Property(TJS_W(# name), &get, &set, TrueTagT())
#define NCB_PROPERTY_PROXY_RO(name,get) /*                     */_r_.Property(TJS_W(# name), &ClassT::get, (int)0, TrueTagT())
#define NCB_PROPERTY_PROXY_WO(name,set) /*                     */_r_.Property(TJS_W(# name), (int)0, &ClassT::set, TrueTagT())
#define NCB_PROPERTY_PROXY_DETAIL(name, r_tag, r_result, r_method, r_args, w_tag, w_result, w_method, w_args) \
	_r_.Property(TJS_W(# name), NCB_METHOD_CAST(r_tag, r_result, r_method, r_args), NCB_METHOD_CAST(w_tag, w_result, w_method, w_args), TrueTagT())
#define NCB_PROPERTY_PROXY_DETAIL_RO(name, tag, result, method, args) _r_.Property(TJS_W(# name), NCB_METHOD_CAST(tag, result, method, args), (int)0, TrueTagT())
#define NCB_PROPERTY_PROXY_DETAIL_WO(name, tag, result, method, args) _r_.Property(TJS_W(# name), (int)0, NCB_METHOD_CAST(tag, result, method, args), TrueTagT())

////////////////////////////////////////
/// TJS�t�@���N�V�����������W�X�g�N���X

struct  ncbNativeFunctionAutoRegister : public ncbAutoRegister {
	ncbNativeFunctionAutoRegister() : ncbAutoRegister(ClassRegist) {}
protected:
	template <typename MethodT>
	static void RegistFunction(NameT name, NameT attach, MethodT m) {
		typedef ncbNativeClassMethod<void, MethodT> MethodObjectT;
		iTJSDispatch2 *dsp = GetDispatch(attach);
		if (!dsp) return;
		MethodObjectT *mobj = new MethodObjectT(m);
		if (mobj) {
			tTJSVariant val(mobj);
			mobj->Release();
			dsp->PropSet(TJS_MEMBERENSURE, name, 0, &val, dsp);
		}
		dsp->Release();
	}

	static void UnregistFunction(NameT name, NameT attach) {
		iTJSDispatch2 *dsp = GetDispatch(attach);
		if (!dsp) return;
		dsp->DeleteMember(0, name, 0, dsp);
		dsp->Release();
	}

private:
	static iTJSDispatch2* GetDispatch(NameT attach) {
		iTJSDispatch2 *ret, *global = TVPGetScriptDispatch();
		if (!global) return 0;

		if (!attach) ret = global;
		else {
			tTJSVariant val;
			global->PropGet(0, attach, 0, &val, global);
			global->Release();
			ret = val.AsObject();
		}
		return ret;
	}
};
template <typename DUMMY>
struct ncbNativeFunctionAutoRegisterTempl;

#define NCB_REGISTER_FUNCTION_COMMON(name, attach, function) \
	struct ncbFunctionTag_ ## name {}; \
	template <> struct ncbNativeFunctionAutoRegisterTempl<ncbFunctionTag_ ## name> : public ncbNativeFunctionAutoRegister \
	{	void Regist()   const { RegistFunction(TJS_W(# name), attach, &function); } \
		void Unregist() const { UnregistFunction(TJS_W(# name), attach); } }; \
	static ncbNativeFunctionAutoRegisterTempl<ncbFunctionTag_ ## name> ncbFunctionAutoRegister_ ## name

#define NCB_REGISTER_FUNCTION(name, function)       NCB_REGISTER_FUNCTION_COMMON(name, 0, function)
#define NCB_ATTACH_FUNCTION(name, attach, function) NCB_REGISTER_FUNCTION_COMMON(name, TJS_W(# attach), function)




////////////////////////////////////////
/// ���W�X�g�O��̃R�[���o�b�N�o�^
struct ncbCallbackAutoRegister : public ncbAutoRegister {
	typedef void (*CallbackT)();

	ncbCallbackAutoRegister(LineT line, CallbackT init, CallbackT term)
		: ncbAutoRegister(line), _init(init), _term(term) {}
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
