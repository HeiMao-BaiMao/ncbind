Title: �g���g���v���O�C�������l�C�e�B�u�N���X�o�C���_
Author: miahmie


������͉����H

������ C++ ���C�u������ TJS �ň������߂̃v���O�C����
�ȒP�ɍ�邱�Ƃ��ł���悤�ɂ��邽�߂� C++ �e���v���[�g�ł��B

�]���̃v���O�C���\�[�X�ł͈����̎󂯓n�������̃��b�p��
tp_stub.h �̃}�N�����g����ŏ����Ȃ���΂Ȃ�Ȃ������̂ł����C
�e���v���[�g�ɂ�肱����قƂ�ǃR���p�C���ɔC���邱�Ƃ��ł��܂��B

gcc 3.4 �n��� VC++2005 Express �œ���m�F���Ă��܂��B
�e���v���[�g�����̊֌W�� VC++6 �ł̓R���p�C�����疳�����Ǝv���܂��B
VC++2003 �⑼�̃R���p�C���͖��m�F�Ȃ̂łǂ��Ȃ邩�킩��܂���B


���ȒP�Ȑ���

�}�N���œo�^�������N���X�̒�`���L�q���邾���ł��B

#include "ncbind.hpp"
#include "�������C�u�����̃w�b�_��"

NCB_REGISTER_CLASS(�o�^����N���X��) {
  NCB_CONSTRUCTOR((�R���X�g���N�^�̈����̌^ ...));
  NCB_METHOD(�o�^���郁�\�b�h1);
  NCB_METHOD(�o�^���郁�\�b�h2);
     :
  NCB_METHOD(�o�^���郁�\�b�hn);
}

�ڂ����� testbind.cpp �̃\�[�X������Ȃǂ��Ă��������B


���t�@�C���\��

  ncbind.hpp		���C���e���v���[�g
  ncbind.cpp		V2Link/V2Unlink ��`
  ncbind.def		gcc�p�G�N�X�|�[�g�t�@�C��
			VC++�ł� /EXPORT:V2Link /EXPORT:V2Unlink ���Ă�������
  ncb_invoke.hpp	�C�ӂ̃��\�b�h���ĂԂ��߂̃e���v���[�g
  ncb_foreach.h		�}�N����C�ӌ��W�J���邽�߂�include�}�N��

  testbind.*		�e�X�g�p�̃v���W�F�N�g


���ڍ�


�@��NCB_REGISTER_CLASS(Class) { ... }
�@��NCB_REGISTER_CLASS_DIFFER(Name, Class) { ... }

Class �� TJS�O���[�o�����(Name or Class)�ɓo�^���܂��B
NCB_REGISTER_CLASS �ł� Class ���CNCB_REGISTER_CLASS_DIFFER�ł� Name ��
�O���[�o����ԏ�ł̖��O�ƂȂ�܂��B�����N���X�̑��d�o�^�̓G���[�ɂȂ�܂��B


�ȉ��� NCB_{ CONSTRUCTOR, METHOD*, PROPERTY* }�}�N����
���� NCB_REGISTER_CLASS �u���b�N���ł����g���܂���B


�@�@��NCB_CONSTRUCTOR((arg1, arg2, ...));

�����̌^�̃��X�g (arg1, arg2, ...) ��n���ăR���X�g���N�^��o�^���܂��B
�^�̈�v�����R���X�g���N�^�� TJS ���� new ����Ƃ��ɌĂ΂�܂��B
�o�^���Ȃ��ꍇ�͂��̃N���X�� new ����ƃG���[�ɂȂ�܂��B�i���܂��������j
�܂��C�I�[�o�[���[�h���ꂽ�����R���X�g���N�^�̓o�^�͂ł��܂���B
�ŏ��ɓo�^���ꂽ���̂��L���ƂȂ�܂��B�i���s���Ɍx�����b�Z�[�W���o�܂��j


�@�@��NCB_METHOD(Method);

�N���X���\�b�h Method ��o�^���܂��B
Method �����̂܂܃N���X���\�b�h���ƂȂ�܂��B
�I�[�o�[���[�h���œ����O�̃��\�b�h����������ꍇ��
���\�b�h�^�̎�������Ɏ��s����̂� NCB_METHOD_DETAIL ���g���Ă��������B


�@�@��NCB_METHOD_DIFFER(Name, Method);

Name �Ƃ������O�ŃN���X���\�b�h Method ��o�^���܂��B
NCB_METHOD(Method) �� NCB_METHOD_DIFFER(Method, Method) �Ɠ����ł��B


�@�@��NCB_METHOD_DETAIL(Name, Type, ReturnType, Method, (arg1,arg2, ...));

������Ԃ�l���w�肵�ă��\�b�h Method �� Name �Ƃ������O�œo�^���܂��B
�ΏۃN���X���̃��\�b�h�ł���ꍇ�� ClassT::Method �ƋL�q���ēn���Ă��������B
�iMethod �͑ΏۃN���X�O�� static �ȃ��\�b�h���n���܂��B�j

Type �̓N���X���\�b�h�̃^�C�v�ŁC
	Class	���ʂ̃N���X���\�b�h
	Const	const �ȃN���X���\�b�h�iReturnType Method(arg...) const;�j
	Static	static �ȃN���X���\�b�h
�� 3�̂����ǂꂩ���L�q���܂��B


�@�@��NCB_METHOD_PROXY(Name, Method);
�@�@��NCB_METHOD_PROXY_DETAIL(Name, ...); // METHOD_DETAIL�Ɠ���

�N���X�O��static�֐����N���X���\�b�h�Ƃ��ĐU���킹��悤�o�^���܂��B
���̊֐��̈�Ԗڂ̈����͕K�����̃N���X�̃C���X�^���X�|�C���^�^�ɂ��܂��B
����� this �����̒l���n��CTJS����n�����c��̈����͂��̌�ɕ��ׂ܂��B

�����̃��C�u������o�^����Ƃ��ɁC���C�u�����Ɏ��������
���炩�̓���ȏ�������ꂽ���Ȃǂ̏ꍇ�ŗL���ł��B


�@�@��NCB_METHOD_RAW_CALLBACK(Name, Callback, Flag);

�R�[���o�b�N���w�肵�� Name �Ƃ������O�Ń��\�b�h��o�^���܂��B
Callback �� tTJSNativeClassMethodCallback �^�� static �֐��|�C���^���C
�܂��́CtTJSNativeClassMethodCallback �̈����� iTJSDispatch2 *objthis ��
���̃N���X�̃C���X�^���X�̃|�C���^�ɂ������̂��g���܂��B
�i���̏ꍇ�C���ۂ̃l�C�e�B�u�C���X�^���X�ւ̃|�C���^�������Ƃ��ēn��܂��j

Flag �́C
	0		 (�ʏ�N���X���\�b�h��)
	TJS_STATICMEMBER (static���\�b�h��) 
�̂ǂ��炩���w��ł��܂��B


�@�@��NCB_PROPERTY   (Property, GetterMethod, SetterMethod);
�@�@��NCB_PROPERTY_RO(Property, GetterMethod);
�@�@��NCB_PROPERTY_WO(Property, SetterMethod);

Property �Ƃ������O�Ńv���p�e�B��o�^���܂��B
GetterMethod, SetterMethod �͂��ꂼ��
�v���p�e�B�擾�C�v���p�e�B�ݒ胁�\�b�h�ł��B
_RO, _WO �͓ǂݍ��ݐ�p�C�������ݐ�p�v���p�e�B�����Ƃ��Ɏg���܂��B

Setter/Getter�̃��\�b�h�^�̃`�F�b�N���Â��̂�
�w�肷�郁�\�b�h�ɂ͒��ӂ��Ă��������B

�@�@��NCB_PROPERTY_DETAIL   (Name, Type, ReturnType, Method, (args, ...),
�@�@                               Type, ReturnType, Method, (args, ...));
�@�@��NCB_PROPERTY_DETAIL_RO(Name, Type, ReturnType, Method, (args, ...));
�@�@��NCB_PROPERTY_DETAIL_RW(Name, Type, ReturnType, Method, (args, ...));

�֐��̌^���w�肵�ăv���p�e�B��o�^���܂��B

�@�@��NCB_PROPERTY_PROXY,_RO,_WO
�@�@��NCB_PROPERTY_PROXY_DETAIL,_RO,_WO

NCB_METHOD_PROXY �̃v���p�e�B�łł��B



�@��NCB_ATTACH_CLASS(          Class, TJS2Class) { ... }
�@��NCB_ATTACH_CLASS_WITH_HOOK(Class, TJS2Class) { ... }

�g���g���Q�̊����̃N���X TJS2Class �� Class ��t�����܂��B
{ ... } �X�R�[�v���� NCB_REGISTER_CLASS �Ɠ����悤�ɒ�`���܂��B
�������C�R���X�g���N�^��` NCB_CONSTRUCTOR �͎g���܂���B

NCB_ATTACH_CLASS �œo�^�����ꍇ�CClass �̃C���X�^���X��
�o�^���ꂽ Class �̃��\�b�h�����߂ČĂ΂ꂽ�Ƃ���
�����Ȃ��̃R���X�g���N�^�� new ����C���\�b�h���Ă΂�܂��B

NCB_ATTACH_CLASS_WITH_HOOK �œo�^����ꍇ�́C���炩���ߌ�q��
NCB_GET_INSTANCE_HOOK ����`�ς݂łȂ���΂Ȃ�܂���B
Class �C���X�^���X�̐����̓t�b�N�p�N���X�Ɉ�C����܂��B


�@��NCB_GET_INSTANCE_HOOK(Class) { ... };
�@�@��NCB_GET_INSTANCE_HOOK_CLASS
�@�@��NCB_INSTANCE_GETTER(ObjThis)

TJS ����l�C�e�B�u�N���X Class �̃��\�b�h���Ăяo������
�C���X�^���X���擾����֐����t�b�N�i�Ƃ������Ē�`�j���܂��B
�ڂ����� testbind.cpp �\�[�X���Q�Ƃ��Ă��������B

NCB_ATTACH_CLASS �����łȂ��CNCB_REGISTER_CLASS ��
�o�^�����N���X�ɂ��K�p����܂��B

�܂��C���ׂẴN���X���\�b�h�ɓK�p����邽�߁C
�w��̌ʃ��\�b�h�Ƀt�b�N���铙�͂ł��܂���B
�@�˗v�]������Ύ������܂�



�@��NCB_REGISTER_FUNCTION(Name, Function);

TJS �O���[�o����Ԃ� Name �Ƃ������O�� Function �Ƃ����֐���o�^���܂��B


�@��NCB_ATTACH_FUNCTION(Name, TJS2Class, Function);

�g���g���Q�̊����̃N���X TJS2Class �� Name �Ƃ������O��
Function ��t�����܂��BFunction �� tTJSNativeClassMethodCallback �^�̏ꍇ��
���̂܂܃��\�b�h�� RawCallback �Ɠ����悤�ɓ��삵�܂��B

���̃}�N���ł́C�� static �Ȋ֐������t���ł��܂���Bstatic �ȃ��\�b�h�Ƃ���
�o�^�������ꍇ�́C�_�~�[�� static ���\�b�h�������݂��Ȃ��N���X�����C
NCB_ATTACH_CLASS �œo�^���Ă��������B�istatic ���\�b�h�̌Ăяo���ł����
�l�C�e�B�u�C���X�^���X�͐�������܂���j



�@��NCB_REGISTER_INSTANCE(...); //���܂�������


�@��NCB_TYPECONV_CAST(Type, CastType);

�����̌^�� Type �̏ꍇ�C�L���X�g CastType ���w�肵��
tTJSVariant �Ƒ��ݕϊ�����悤�ɓo�^���܂��B

�@��NCB_SET_CONVERTOR(Type, Convertor);

�����̌^ Type ��ϊ�����N���X��o�^���܂��B
�ڂ����� ncbind.hpp �̃R�����g�����Q�Ƃ��Ă��������B



�@��NCB_PRE_REGIST_CALLBACK(Callback);
�@��NCB_POST_REGIST_CALLBACK(Callback);
�@��NCB_PRE_UNREGIST_CALLBACK(Callback);
�@��NCB_POST_UNREGIST_CALLBACK(Callback);

�N���X��o�^�E�J������O��ɌĂ΂��R�[���o�b�N void Callback() ��
�o�^���܂��B�Ă΂�鏇�Ԃ͈ȉ��̂Ƃ���ł��B

V2Link���F
	PRE_REGIST_CALLBACK
	�N���X�o�^
	POST_REGIST_CALLBACK
V2Unlink���F
	PRE_UNREGIST_CALLBACK
	�N���X�J��
	POST_UNREGIST_CALLBACK

������ނ̃R�[���o�b�N�������o�^���ꂽ�ꍇ�̏��Ԃ̕ۏ؂͂���܂���B



������

�E�p���֌W�̓T�|�[�g���Ă��Ȃ�
�@�ˌp���֌W�ɂ���N���X���m��o�^�����ꍇ�C�ʂ̃N���X�����ɂȂ�
	�Einstanceof �Ŕh���N���X�C���X�^���X�̃`�F�b�N���ł��Ȃ�
	�E�����ɔh���N���X�C���X�^���X��n�����ꍇ��
	�@�C���X�^���X�|�C���^���擾�ł����ɃG���[�ƂȂ�

�E�����̏ȗ��ɂ��f�t�H���g�l���T�|�[�g���Ă��Ȃ�
�@��TJS����n�������̌��̓��\�b�h�̈����̌��ȏ�ł��邱��
�@�@�]���ɓn���ꂽ�����͖�������C����Ȃ��ꍇ�� TJS_E_BADPARAMCOUNT ���Ԃ�
�@�ˉϒ��������T�|�[�g����ꍇ�� RawCallback �Ŏ��͎������邱��

�Enamespace ���ł̃N���X�o�^�͍l�����Ă��Ȃ��̂� namespace �O�ōs������
�@��namespace ���̃N���X��o�^����ꍇ�� typedef ����Ȃǂ�
�@�@�K�� namespace �̊O�ōs�� (:: ���܂ނƃG���[�ɂȂ�)

  ex.
	typedef ::Foo::Bar::TargetClass TargetClass;

	NCB_REGISTER_CLASS(TargetClass) { ... }


�E�R���X�g���N�^�͂P�����L�q�ł��Ȃ�
�@�˃C���X�^���X�𐶐����� static �ȃ��\�b�h�������Ȃǂ��ĉ�������

�Estatic �Ȋ����̃C���X�^���X�Ȃǂ�Ԃ�l�œn���Ă͂����Ȃ�
�@�iTJS���� invalidate �����Ƃ��ɃC���X�^���X�� delete ����邽�߁j

�E�Q�ƂŒl�����������ĕԂ��悤�ȃ��\�b�h�ɂ͑Ή��ł��Ȃ�
�@�˓K����RawCallback�֐��������Ȃǂ��đΏ����邱��

�ETJSCreateNativeClassMethod ���g�킸�Ƀ��\�b�h�Ăяo����Ǝ�����
�@���Ă��邽�߁C�����ɂ킽��\�[�X�݊����ۂĂ�Ƃ����ۏ؂��Ȃ�
�@�ˋg���g���Q���̂��������e�t�F�[�Y������ʂɂ����c��ˁH


���Ƃ茾

�E�e���v���[�g�� Modern C++ Design ���Ђ�ǂ񂾂��炢�ŁCBoost�Ƃ����
�@ncb_invoke�̑O�g�o�[�W������TypeList�Ŏ��������������������
�@�����܂ł���悤�ȕ��ł͂Ȃ��ƌ�����̂ō���͗͋ƂŎ���

�EtTJSVariantType tTJSVariant::Type() �͉��� const ���\�b�h�łȂ��̂�
�@const_cast�g���n���ɂȂ��Ĕs�k�C��
	�ˏC������܂������C�]���݊��d���̂��ߌ��ǎg���Ȃ��i�܁j



��TODO����

�Encibind.hpp �Â��R�����g�̐���
�ENCB_SET_CONVERTOR �e�X�g
�Edelete����Ȃ��A�_�v�^�� REGISTER_INSTANCE
�EAttach���Ɋ����̃��\�b�h���������ꍇ�̏���
�E�I�[�o�[���[�h�ƈ����̏ȗ����T�|�[�g���邩�H
�E���\�b�h���Ă΂��O��̃t�b�N���L�q����}�N��
	��ncbNativeClassMethodBase::invokeHookClass, invokeHookAll
