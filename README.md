# DDSTestSubscriber

This C++ application demonstrates the subscription of a DDS application to DDS topics. In this case, these topics correspond to
notifications (constants, data, events) sent by an OPC UA server and relayed by the Neoliant "OPC UA - DDS Gateway".

Furthermore, this application demonstrates the integration of the Neoliant "libopcddsservices" library so that it can transparently call
services deployed on an OPC UA server. These typical RPC calls are relayed by the OPC UA - DDS Gateway, with automatic conversions in between the Request / Reply pattern
and the Publish / Subscribe one. The Neoliant library offers synchronous and asynchronous implementations for the following OPC UA services :

* Attribute
* View
* Query
* Method

You can find a [complete documentation](https://neoliant.com/docs/tutorials.html#compiling-the-code-of-the-demo-dds-application) on the [Neoliant website](https://neoliant.com) describing the compilation and the execution of DDSTestSubscriber demo together with
an OPC UA test server and the OPC UA - DDS Gateway.

