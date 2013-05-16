package com.aldebaran.qimessaging;

import java.util.Hashtable;
import java.util.Map;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TupleTest
{
  public GenericObject    proxy = null;
  public GenericObject    obj = null;
  public Session          s = null;
  public Session          client = null;
  public ServiceDirectory sd = null;

  @Before
  public void setUp() throws Exception
  {
    sd = new ServiceDirectory();
    s = new Session();
    client = new Session();

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Create new QiMessaging generic object
    obj = new GenericObject();

    // Get instance of ReplyService
    QimessagingService reply = new ReplyService();

    // Register event 'Fire'
    obj.advertiseSignal("fire::(i)");
    obj.advertiseMethod("reply::s(s)", reply);
    obj.advertiseMethod("answer::s()", reply);
    obj.advertiseMethod("add::i(iii)", reply);
    obj.advertiseMethod("info::(sib)(sib)", reply);

    // Connect session to Service Directory
    s.connect(url).sync();

    // Register service as serviceTest
    assertTrue("Service must be registered", s.registerService("serviceTest", obj));

    // Connect client session to service directory
    client.connect(url);

    // Get a proxy to serviceTest
    proxy = client.service("serviceTest");
    assertNotNull(proxy);
  }

  @After
  public void tearDown()
  {
    obj = null;
    proxy = null;

    s.close();
    client.close();

    s = null;
    client = null;
    sd = null;
  }

  @Test
  public void testOutOfBoundException()
  {
    Tuple tuple = new Tuple3<String, Integer, Map<Integer, String>>();
    boolean exceptionRaised = false;

    try
    {
      tuple.<String>get(4);
    } catch (Exception e)
    {
      exceptionRaised = true;
      System.out.println("Exception catched : " + e.getMessage());
    }

    assertTrue("Exception must have been thrown", exceptionRaised);
  }

  @Test
  public void testTuple()
  {
    Tuple tuple = new Tuple3<String, Integer, Boolean>();
    String str  = new String("plafbim");
    Integer i   = new Integer(42);
    Boolean b   = new Boolean(true);

    try
    {
      tuple.<String>set(0, str);
    } catch (Exception e)
    {
      fail("Exception must not be thrown (string)");
    }

    try
    {
      tuple.<Integer>set(1, i);
    } catch (Exception e)
    {
      fail("Exception must not be thrown (integer)");
    }

    try
    {
      tuple.<Boolean>set(2, b);
    } catch (Exception e)
    {
      fail("Exception must not be thrown (boolean)");
    }

  }

  @Test
  public void testValue()
  {
    Tuple tuple = new Tuple3<String, Integer, Boolean>("42", 42, true);
    String str  = null;
    Integer i   = null;
    Boolean b   = null;

    try
    {
      str = tuple.<String>get(0);
    } catch (Exception e)
    {
      fail("Exception must not be thrown : " + e.getMessage());
    }

    assertEquals("42", str);

    try
    {
      i = tuple.<Integer>get(1);
    } catch (Exception e)
    {
      fail("Exception must not be thrown : " + e.getMessage());
    }

    assertEquals(42, i.intValue());

    try
    {
      b = tuple.<Boolean>get(2);
    } catch (Exception e)
    {
      fail("Exception must not be thrown : " + e.getMessage());
    }

    assertTrue("Boolean value must be true : " + b, b);
  }

  @Test(expected=IndexOutOfBoundsException.class)
  public void testIndexOutOfBoundsException() throws IndexOutOfBoundsException, ClassCastException, IllegalArgumentException, IllegalAccessException
  {
    Tuple tuple = new Tuple3<String, Integer, Map<Integer, String>>("42", 42, null);

    tuple.<String>get(42);
  }


  @Test(expected=ClassCastException.class)
  public void testClassCastException() throws IndexOutOfBoundsException, ClassCastException, IllegalArgumentException, IllegalAccessException
  {
    Tuple tuple = new Tuple3<String, Integer, Map<Integer, String>>("42", 42, new Hashtable<Integer, String>());

    @SuppressWarnings("unused")
    ReplyService tmp = tuple.<ReplyService>get(0);
  }

  @Test(expected=IllegalArgumentException.class)
  public void testIllegalArgumentException() throws IndexOutOfBoundsException, ClassCastException, IllegalArgumentException, IllegalAccessException
  {
    Tuple tuple = new Tuple3<String, Float, Map<Integer, String>>("42", 42F, new Hashtable<Integer, String>());

    tuple.<Long>set(0, new Long(1234567890));
    tuple.<Long>set(1, new Long(1234567890));
    tuple.<Long>set(2, new Long(1234567890));
  }

  @Test
  public void testCallTuple()
  {

    Tuple ret = null;
    try
    {
      ret = proxy.<Tuple>call("info", "42", 42, true);
    } catch (CallError e)
    {
      fail("Call must be successful : " + e.getMessage());
    }

    String str = null;
    Integer i  = null;
    Boolean b  = null;

    try
    {
      str = ret.<String>get(0);
    } catch (Exception e)
    {
      fail("Exception must not be thrown : " + e.getMessage());
    }

    try
    {
      i = ret.<Integer>get(1);
    } catch (Exception e)
    {
      fail("Exception must not be thrown : " + e.getMessage());
    }

    try
    {
      b = ret.<Boolean>get(2);
    } catch (Exception e)
    {
      fail("Exception must not be thrown : " + e.getMessage());
    }

    assertEquals("42", str);
    assertEquals(42, i.intValue());
    assertTrue(b);
  }

}
