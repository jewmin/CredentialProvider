ʹ��˵��

1����װƾ֤
�����x86ϵͳ������Win32��dll��C:\Windows\System32Ŀ¼
�����x64ϵͳ������x64��dll��C:\Windows\System32Ŀ¼
����ԱȨ������Register.reg

2��жװƾ֤
����ԱȨ������Unregister.reg

3����װ����
ʹ�ù���ԱȨ������cmd����cmd��ִ��ָ��DemoService.exe --install��Ȼ����ڷ���������ҵ���Ӧ����Demo Service
ע1����װʱ�����ʹ��ȫ·������ȻĬ��·����C:\Windows\System32����Ҫ��DemoService.exe���Ƶ�C:\Windows\System32Ŀ¼��������
ע2��win+r�����п�����services.msc���ɴ򿪷������������������Demo Service��˫���򿪺�������������ֱ������ϵͳ���Զ�����

4��жװ����
ʹ�ù���ԱȨ������cmd����cmd��ִ��ָ��DemoService.exe --uninstall

5��ƾ֤����
1.ϵͳ����CSampleProviderFilter::Filter����������ƾ֤��ֻ�����Լ���ƾ֤������������Σ����԰�Register.reg�����������ɾ����
HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Provider Filters\{FF032558-38DA-4472-B969-31A636B7E5C7}
ע��FF032558-38DA-4472-B969-31A636B7E5C7�����ƾ֤��ΨһID����Ҫ��guid.h��ֵһ��
DEFINE_GUID(CLSID_CSample, 0xff032558, 0x38da, 0x4472, 0xb9, 0x69, 0x31, 0xa6, 0x36, 0xb7, 0xe5, 0xc7);

2.ϵͳ����CSampleProvider::SetUsageScenario����ʼ��ƾ֤

3.ϵͳ����CSampleCredential::Initialize����ʼ��ƾ֤�еĿؼ�

4.���������û��������룬Ȼ����ȷ����ť��ϵͳ����CSampleCredential::Connect���������������̣߳��ȴ���Ȩ��֤�������

5.ϵͳ����CSampleCredential::GetSerialization���������ϵͳ��¼��֤�������Ȩʧ�ܣ���ֱ�ӷ��أ���������¼��֤

6.ϵͳ����CSampleCredential::ReportResult�������¼ʧ�ܣ��������ﴦ��

6��ƾ֤�ؼ�
1��CSampleCredential::GetBitmapValue�����������޸�ͼƬ
2��CSampleCredential::CommandLinkClicked�����������޸ĳ����ӣ�Ĭ��������ʾMessageBox���ɸĳɴ����Ի���

7��ע������
1��Win10����������£��ػ������������Ĺػ����������ߡ���������ϵͳ����ʾ�ĵ�¼���棬��ʵ���ڽ����û������ǵ�¼�û���������Service_xxx.log��־�й۲쵽
2��MyService�ǲ���ҵ���߼��ģ�Ӧ����������д
3�����·����汾������ʱ����Ҫ�������û��������룬��Ϊ���ص���Ȩ�����������ɵģ�������ε�¼����
4�������������޷�ֹͣ�����Դ�������������ҵ���Ӧ�ķ�����̣�ǿ�ƽ�������
5����־������C:\Windows\System32